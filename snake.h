#ifndef SNAKE_H
#define SNAKE_H

#define BOARD_WIDTH 40
#define BOARD_HEIGHT 20
#define INITIAL_SNAKE_LENGTH 3
#define MAX_SNAKE_LENGTH ((BOARD_WIDTH - 2) * (BOARD_HEIGHT - 2))
#define SCORE_PER_FOOD 10
#define TRAP_START_LENGTH 10
#define TRAP_LENGTH_INTERVAL 3
#define MAX_TRAPS 12
#define DIFFICULTY_COUNT 5
#define MAX_NAME_BYTES 40
#define MAX_TIME_BYTES 24
#define MAX_RECORDS 500

#include <stddef.h>

typedef struct
{
    int x;
    int y;
} Point;

typedef enum
{
    DIR_UP,
    DIR_RIGHT,
    DIR_DOWN,
    DIR_LEFT,
    DIR_NONE
} Direction;

typedef struct
{
    Point body[MAX_SNAKE_LENGTH];
    Direction direction;
    int length;
} Snake;

typedef enum
{
    GAME_RUNNING,
    GAME_LOST,
    GAME_WON
} GameState;

typedef enum
{
    STEP_CONTINUE,
    STEP_LOST,
    STEP_WON
} StepResult;

typedef struct
{
    Snake snake;
    Point food;
    Point traps[MAX_TRAPS];
    int trap_count;
    int score;
    int difficulty;
    int has_food;
    GameState state;
} Game;

typedef struct
{
    char name[MAX_NAME_BYTES];
    int score;
    int difficulty;
    char time[MAX_TIME_BYTES];
} GameRecord;

typedef struct
{
    GameRecord items[MAX_RECORDS];
    int count;
} RecordList;

const char *difficulty_name(int difficulty);
int difficulty_delay(int difficulty);
int game_init(Game *game, int difficulty);
int game_try_set_direction(Game *game, Direction direction);
StepResult game_update(Game *game);
void game_render(const Game *game, int best_score);
int game_is_occupied(const Game *game, int x, int y);
int game_is_trap(const Game *game, int x, int y);
int game_trap_target(int snake_length);
int game_is_full(const Game *game);
int run_self_tests(void);

#endif
