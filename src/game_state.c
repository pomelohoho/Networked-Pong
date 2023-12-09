#include "game_state.h"  
#include <curses.h>
#include <stdbool.h>
#include "scheduler.h"
#include "util.h"

#define MAX_HEIGHT 15
#define MAX_WIDTH 40
#define WINNING_SCORE 5  // The game ends when a player reaches this score
#define PADDLE_SIZE 3  // Increase the paddle size

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
    } else if (state->paddle1_y > MAX_HEIGHT - PADDLE_SIZE) {
        state->paddle1_y = MAX_HEIGHT - PADDLE_SIZE;
    }

    if (state->paddle2_y < 1) {
        state->paddle2_y = 1;
    } else if (state->paddle2_y > MAX_HEIGHT - PADDLE_SIZE) {
        state->paddle2_y = MAX_HEIGHT - PADDLE_SIZE;
    }

    // Check for collisions with paddles
    for (int i = 0; i < PADDLE_SIZE; i++) {
        if (state->ball_x <= 2 && (int)state->ball_y == state->paddle1_y + i) {
            state->ball_dx = -state->ball_dx; // Reverse ball direction
            break;
        }
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
