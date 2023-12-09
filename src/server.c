#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "scheduler.h"
#include "util.h"
#include "socket.h"
#include "message.h"

#define MAX_HEIGHT 15
#define MAX_WIDTH 40
#define WINNING_SCORE 5  // The game ends when a player reaches this score
#define PADDLE_SIZE 3  // Increase the paddle size

typedef struct {
    float paddle1_y, paddle2_y;
    float ball_x, ball_y;
    float ball_dx, ball_dy;
    int score1, score2;
} GameState;

GameState state;

void init_game_state(GameState *state);
void update_game_state(GameState *state);
void render_game(const GameState *state);
void game_loop();

void serialize_game_state(const GameState *state, char *buffer) {
    sprintf(buffer, "%f,%f,%f,%f,%d,%d",
            state->paddle1_y, state->paddle2_y,
            state->ball_x, state->ball_y,
            state->score1, state->score2);
}
int main() {
    unsigned short port = 0;
    int server_socket_fd = server_socket_open(&port);
    if (server_socket_fd == -1) {
        perror("Server socket was not opened");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket_fd, 1)) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %u\n", port);

    while (1) {
        int client_socket_fd = server_socket_accept(server_socket_fd);
        if (client_socket_fd == -1) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected!\n");
             // Initialize game state
        init_game_state(&state);
        
        while (1) {
            update_game_state(&state);
            // Serialize and send the game state back to the client
            char buffer[1024]; // Define an adequate buffer size
            serialize_game_state(&state, buffer);
            // Debug message: print the serialized game state
            printf("Serialized GameState: %s\n", buffer);
            send_message(client_socket_fd, buffer);
            char* message = receive_message(client_socket_fd);
            if (message == NULL) break;
            
             // Trim newline character at the end of the message
            message[strcspn(message, "\n")] = 0;

            // Process message for paddle control
            if (strcmp(message, "w") == 0) {
                state.paddle1_y--;
                printf("paddle pos: %f\n", state.paddle1_y);
                printf("ball: %f\n", state.ball_x);

            } else if (strcmp(message, "s") == 0) {
                state.paddle1_y++;\
                printf("paddle pos: %f\n", state.paddle1_y);
                printf("ball: %f\n", state.ball_x);
                
            }

            if (state.score1 >= WINNING_SCORE || state.score2 >= WINNING_SCORE) {
                break;  // End the game loop
            }
    
            free(message);
        }

        printf("Client disconnected.\n");
        close(client_socket_fd);
    }
    // Display the winner
    if (state.score1 >= WINNING_SCORE) {
        printf("Player 1 wins!\n");
    } else {
        printf("Player 2 wins!\n");
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