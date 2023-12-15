#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include "socket.h"
#include "message.h"
#include "game_state.h"
#include "util.h"

#define MAX_CLIENTS 2

GameState state;
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
int client_sockets[MAX_CLIENTS] = {0};
int connected_clients = 0;

void* handle_client(void* arg);
void* game_update_loop(void* arg);

int main() {
    unsigned short port = 0;
    int server_socket_fd = server_socket_open(&port);
    if (server_socket_fd == -1) {
        perror("Server socket was not opened");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket_fd, MAX_CLIENTS)) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %u\n", port);

    init_game_state(&state);

    // Start the game update thread
    pthread_t game_update_thread;
    if (pthread_create(&game_update_thread, NULL, game_update_loop, NULL) != 0) {
        perror("Failed to create game update thread");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int* client_socket_fd = malloc(sizeof(int));
        *client_socket_fd = server_socket_accept(server_socket_fd);
        if (*client_socket_fd == -1) {
            perror("Accept failed");
            free(client_socket_fd);
            continue;
        }

        pthread_mutex_lock(&state_mutex);
        // for (int i = 0; i < MAX_CLIENTS; i++) {
        //     if (client_sockets[i] != 0) {
        //         connected_clients++;
        //     }
        // }

        if (connected_clients >= MAX_CLIENTS) {
            printf("Maximum clients reached. Rejecting client %d.\n", *client_socket_fd);
            send_message(*client_socket_fd, "Server is full. Connection rejected.");
            close(*client_socket_fd);
            free(client_socket_fd);
        } else {
            pthread_t thread_id;
            if (pthread_create(&thread_id, NULL, handle_client, client_socket_fd) != 0) {
                perror("Failed to create thread");
                close(*client_socket_fd);
                free(client_socket_fd);
            }
        }
        pthread_mutex_unlock(&state_mutex);
    }

    close(server_socket_fd);
    return 0;
}

void* handle_client(void* arg) {
    int client_socket_fd = *((int*)arg);
    free(arg);

    pthread_mutex_lock(&state_mutex);
    connected_clients++;  // Increment connected clients
    printf("Client connected! Socket FD: %d\n", client_socket_fd);

    int clientIndex = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == 0) {
            client_sockets[i] = client_socket_fd;
            clientIndex = i;
            break;
        }
    }
    printf("Client will control %s paddle.\n", clientIndex == 0 ? "left" : "right");
    pthread_mutex_unlock(&state_mutex);

    char buffer[1024];
    while (1) {
        char* message = receive_message(client_socket_fd);
        if (message == NULL) break;
        message[strcspn(message, "\n")] = 0;

        pthread_mutex_lock(&state_mutex);
        if (clientIndex == 0) { // Left paddle control
            if (strcmp(message, "w") == 0) state.paddle1_y--;
            if (strcmp(message, "s") == 0) state.paddle1_y++;
        } else if (clientIndex == 1) { // Right paddle control
            if (strcmp(message, "w") == 0) state.paddle2_y--;
            if (strcmp(message, "s") == 0) state.paddle2_y++;
        }
        update_game_state(&state);
        serialize_game_state(&state, buffer);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != 0) {
                send_message(client_sockets[i], buffer);
            }
        }
        pthread_mutex_unlock(&state_mutex);

        free(message);
    }

    pthread_mutex_lock(&state_mutex);
    connected_clients--;  // Decrement connected clients
    printf("Client %d disconnected.\n", client_socket_fd);
    client_sockets[clientIndex] = 0;
    pthread_mutex_unlock(&state_mutex);
    close(client_socket_fd);
    return NULL;
}

void* game_update_loop(void* arg) {
    while (1) {
        pthread_mutex_lock(&state_mutex);
        if (connected_clients == MAX_CLIENTS) {
            update_game_state(&state);
        }
        pthread_mutex_unlock(&state_mutex);

        // Sleep for a short duration to control update rate
        sleep_ms(100); // for example, 10 milliseconds
    }
}