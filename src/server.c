#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include "socket.h"
#include "message.h"

#define MAX_CLIENTS 2
#define MAX_HEIGHT 15
#define MAX_WIDTH 40
#define WINNING_SCORE 5
#define PADDLE_SIZE 3

typedef struct {
    float paddle1_y, paddle2_y;
    float ball_x, ball_y;
    float ball_dx, ball_dy;
    int score1, score2;
} GameState;

GameState state;
pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
int client_sockets[MAX_CLIENTS] = {0};

void init_game_state(GameState *state);
void update_game_state(GameState *state);
void serialize_game_state(const GameState *state, char *buffer);

void* handle_client(void* arg) {
    int client_socket_fd = *((int*)arg);
    free(arg);

    pthread_mutex_lock(&state_mutex);
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
    printf("Client %d disconnected.\n", client_socket_fd);
    client_sockets[clientIndex] = 0;
    pthread_mutex_unlock(&state_mutex);
    close(client_socket_fd);
    return NULL;
}


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

    while (1) {
        int* client_socket_fd = malloc(sizeof(int));
        *client_socket_fd = server_socket_accept(server_socket_fd);
        if (*client_socket_fd == -1) {
            perror("Accept failed");
            free(client_socket_fd);
            continue;
        }

        pthread_mutex_lock(&state_mutex);
        int connected_clients = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] != 0) {
                connected_clients++;
            }
        }

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


void init_game_state(GameState *state) {
    state->paddle1_y = MAX_HEIGHT / 2;
    state->paddle2_y = MAX_HEIGHT / 2;
    state->ball_x = MAX_WIDTH / 2;
    state->ball_y = MAX_HEIGHT / 2;
    state->ball_dx = 1; // Ball movement speed in X
    state->ball_dy = 1; // Ball movement speed in Y
    state->score1 = 0;
    state->score2 = 0;
}


void update_game_state(GameState *state) {
    // Ball positions
    state->ball_x += state->ball_dx;
    state->ball_y += state->ball_dy;

    // Collision with top and bottom walls
    if (state->ball_y <= 0 || state->ball_y >= MAX_HEIGHT - 1) {
        state->ball_dy = -state->ball_dy;
    }
    // Boundary checks for paddles
    if (state->paddle1_y < 1) {
        state->paddle1_y = 1;
    } else if (state->paddle1_y > MAX_HEIGHT - 2) {
        state->paddle1_y = MAX_HEIGHT - 2;
    }

    if (state->paddle2_y < 1) {
        state->paddle2_y = 1;
    } else if (state->paddle2_y > MAX_HEIGHT - 2) {
        state->paddle2_y = MAX_HEIGHT - 2;
    }

    // Check for collisions with left paddle
    for (int i = 0; i < PADDLE_SIZE; i++) {
        if (state->ball_x <= 2 && (int)state->ball_y == state->paddle1_y + i) {
            state->ball_dx = -state->ball_dx; // Reverse ball direction
            break;
        }
    }

    // Check for collisions with right paddle
    for (int i = 0; i < PADDLE_SIZE; i++) {
        if (state->ball_x >= MAX_WIDTH - 3 && (int)state->ball_y == state->paddle2_y + i) {
            state->ball_dx = -state->ball_dx;
            break;
        }
    }

    // Scoring
    if (state->ball_x <= 0) {
        state->score2++;
        state->ball_x = MAX_WIDTH / 2;
        state->ball_y = MAX_HEIGHT / 2;
    } else if (state->ball_x >= MAX_WIDTH - 1) {
        state->score1++;
        state->ball_x = MAX_WIDTH / 2;
        state->ball_y = MAX_HEIGHT / 2;
    }
}

void serialize_game_state(const GameState *state, char *buffer) {
    sprintf(buffer, "%f,%f,%f,%f,%d,%d",
            state->paddle1_y, state->paddle2_y,
            state->ball_x, state->ball_y,
            state->score1, state->score2);
}
