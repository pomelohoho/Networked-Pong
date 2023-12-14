#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "socket.h"
#include "message.h"
#include <curses.h>
#include "util.h"

#define BUFFER_SIZE 1024
#define MAX_HEIGHT 15
#define MAX_WIDTH 70
#define WINNING_SCORE 5  // The game ends when a player reaches this score
#define PADDLE_SIZE 3  // Increase the paddle size

typedef struct {
    float paddle1_y, paddle2_y;
    float ball_x, ball_y;
    float ball_dx, ball_dy;
    int score1, score2;
} GameState;

void init_game_state(GameState *state);
void deserialize_game_state(const char *buffer, GameState *state);
void render_game(const GameState *state);
int receive_game_state(int socket_fd, GameState *state);

int main(int argc, char** argv) {
    // take terminal argument
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server name> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* server_name = argv[1];
    unsigned short port = atoi(argv[2]);
    int socket_fd = socket_connect(server_name, port);
    if (socket_fd == -1) {
        perror("Failed to connect");
        exit(EXIT_FAILURE);
    }

    initscr();            // Initialize the window
    noecho();             // Don't echo inputted keys
    curs_set(FALSE);      // Don't display the cursor
    nodelay(stdscr, TRUE); // Set getch to be a non-blocking call
    
    char buffer[BUFFER_SIZE];
    while (1) {
        // printf("Enter command (w/s): ");
        int ch = getch();  // Get a single character input

        //Clear the input buffer 
        int c;
        while ((c = getch()) != '\n' && c != EOF) { 
            //discard 
        }

        // Check if the input is 'w' or 's'
        // if (ch == 'w' || ch == 's') {
            buffer[0] = ch;
            buffer[1] = '\n';  // Add a newline character
            buffer[2] = '\0';  // Terminate the string
            send_message(socket_fd, buffer);
        // }
        GameState state;
        // init_game_state(&state);

        // Debug prints
        // printf("%d\n", receive_game_state(socket_fd, &state));
        // printf("Initial GameState: Paddle1 Y: %f, Paddle2 Y: %f, Ball X: %f, Ball Y: %f, Score1: %d, Score2: %d\n",
        //    state.paddle1_y, state.paddle2_y, state.ball_x, state.ball_y, state.score1, state.score2);
        
        if (receive_game_state(socket_fd, &state) == 1) {
            render_game(&state);
        }
        
        if (ch == 'q') break; // Quit on 'q'
        sleep_ms(100); // Small delay for smoother animation

    }
    endwin(); // End ncurses mode
    close(socket_fd);
    return 0;
}


void deserialize_game_state(const char *buffer, GameState *state) {
    sscanf(buffer, "%f,%f,%f,%f,%d,%d",
           &state->paddle1_y, &state->paddle2_y,
           &state->ball_x, &state->ball_y,
           &state->score1, &state->score2);
}

void render_game(const GameState *state) {
    clear(); // Clear the screen

    // Draw left paddle
    for (int i = 0; i < PADDLE_SIZE; i++) {
        mvprintw(state->paddle1_y + i, 1, "|");
    }

    // Draw right paddle
    for (int i = 0; i < PADDLE_SIZE; i++) {
        mvprintw(state->paddle2_y + i, MAX_WIDTH - 2, "|");
    }

    // Draw ball
    mvprintw(state->ball_y, state->ball_x, "O");

    // Draw scores
    mvprintw(0, (MAX_WIDTH / 2) - 10, "Score: %d - %d", state->score1, state->score2);

    refresh(); // Refresh the screen to show changes
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

int receive_game_state(int socket_fd, GameState *state) {
    char* serializedState = receive_message(socket_fd);
    if (serializedState == NULL) {
        return 0; // Return 0 on error or no data
    }

    deserialize_game_state(serializedState, state);
    printf("%s\n", serializedState);
    // Debug print to check the game state values
    
    free(serializedState);
    return 1; // Assume successful deserialization
}
