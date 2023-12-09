#ifndef GAME_STATE_H
#define GAME_STATE_H

// Define the structure of your game state
typedef struct {
    float paddle1_y, paddle2_y; // Y positions of paddles
    float ball_x, ball_y;       // Ball position
    float ball_dx, ball_dy;     // Ball velocity in X and Y directions
    int score1, score2;         // Scores for player 1 and player 2
} GameState;

// Function to initialize the game state
void init_game_state(GameState *state);

// Function to update the game state, typically called every frame
void update_game_state(GameState *state);

// Function to serialize the game state into a string
void serialize_game_state(const GameState *state, char *buffer);

// Function to deserialize a string back into a game state
void deserialize_game_state(const char *buffer, GameState *state);

#endif // GAME_STATE_H
