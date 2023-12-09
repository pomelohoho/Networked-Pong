#include <curses.h>
#include <stdbool.h>
#include "scheduler.h"
#include "util.h"

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

int main() {
    initscr();            // Initialize the window
    noecho();             // Don't echo inputted keys
    curs_set(FALSE);      // Don't display the cursor
    nodelay(stdscr, TRUE); // Set getch to be a non-blocking call

    // Initialize game state
    init_game_state(&state);

    // Main game loop
    while (1) {
        int ch = getch(); // Get user input

        // Update game state based on user input
        // 'w' and 's' to move paddle1, Up and Down arrow keys for paddle2
        if (ch == 'w') state.paddle1_y--;
        if (ch == 's') state.paddle1_y++;
        if (ch == KEY_UP) state.paddle2_y--;
        if (ch == KEY_DOWN) state.paddle2_y++;

        update_game_state(&state);
        render_game(&state);
        sleep_ms(100); // Control frame rate
        if (state.score1 >= WINNING_SCORE || state.score2 >= WINNING_SCORE) {
            break;  // End the game loop
        }
    }

    endwin(); // End curses mode
     // Display the winner
    if (state.score1 >= WINNING_SCORE) {
        printf("Player 1 wins!\n");
    } else {
        printf("Player 2 wins!\n");
    }

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
