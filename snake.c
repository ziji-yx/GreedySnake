#define _WIN32_WINNT 0x0600

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>
#include "snake.h"
#include "user.h"
#include "win.h"

#define INFO_ROW (BOARD_HEIGHT + 4)
#define CONTROL_ROW (BOARD_HEIGHT + 5)
#define STATUS_ROW (BOARD_HEIGHT + 6)

static const char *const difficulty_names[DIFFICULTY_COUNT] = {
    "悠闲",
    "轻松",
    "标准",
    "挑战",
    "极限"};

static const int difficulty_speeds[DIFFICULTY_COUNT] = {
    220,
    150,
    100,
    65,
    40};

const char *difficulty_name(int difficulty)
{
    if (difficulty < 0 || difficulty >= DIFFICULTY_COUNT)
    {
        return "标准";
    }
    return difficulty_names[difficulty];
}

int difficulty_delay(int difficulty)
{
    if (difficulty < 0 || difficulty >= DIFFICULTY_COUNT)
    {
        return difficulty_speeds[2];
    }
    return difficulty_speeds[difficulty];
}

static int read_line(char *buffer, size_t capacity)
{
    char *newline;

    if (fgets(buffer, (int)capacity, stdin) == NULL)
    {
        return 0;
    }

    newline = strchr(buffer, '\n');
    if (newline != NULL)
    {
        *newline = '\0';
    }
    else
    {
        int character;
        while ((character = getchar()) != '\n' && character != EOF)
        {
            ;
        }
    }

    return 1;
}

static void write_cleared_line(int row, const char *text)
{
    console_gotoxy(0, row);
    fputs(text, stdout);
    for (int i = 0; i < 80; i++)
    {
        putchar(' ');
    }
}

static void wait_for_enter(void)
{
    char input[16];
    console_show_cursor();
    console_set_color(COLOR_DIM);
    printf("\n按 Enter 返回主菜单...");
    console_set_color(COLOR_DEFAULT);
    (void)read_line(input, sizeof(input));
}

static Point next_head(const Game *game)
{
    Point next = game->snake.body[0];

    switch (game->snake.direction)
    {
    case DIR_UP:
        next.y--;
        break;
    case DIR_DOWN:
        next.y++;
        break;
    case DIR_LEFT:
        next.x--;
        break;
    case DIR_RIGHT:
        next.x++;
        break;
    default:
        break;
    }
    return next;
}

int game_is_occupied(const Game *game, int x, int y)
{
    for (int i = 0; i < game->snake.length; i++)
    {
        if (game->snake.body[i].x == x && game->snake.body[i].y == y)
        {
            return 1;
        }
    }
    return 0;
}

int game_is_trap(const Game *game, int x, int y)
{
    for (int i = 0; i < game->trap_count; i++)
    {
        if (game->traps[i].x == x && game->traps[i].y == y)
        {
            return 1;
        }
    }
    return 0;
}

int game_trap_target(int snake_length)
{
    int count;

    if (snake_length < TRAP_START_LENGTH)
    {
        return 0;
    }

    count = 1 + (snake_length - TRAP_START_LENGTH) / TRAP_LENGTH_INTERVAL;
    return count > MAX_TRAPS ? MAX_TRAPS : count;
}

static int place_trap(Game *game)
{
    int candidates = 0;
    Point selected = {0, 0};
    Point head = game->snake.body[0];

    if (game->trap_count >= MAX_TRAPS)
    {
        return 0;
    }

    for (int y = 1; y < BOARD_HEIGHT - 1; y++)
    {
        for (int x = 1; x < BOARD_WIDTH - 1; x++)
        {
            int distance = abs(x - head.x) + abs(y - head.y);

            if (distance < 3 || game_is_occupied(game, x, y) ||
                game_is_trap(game, x, y) ||
                (game->has_food && game->food.x == x && game->food.y == y))
            {
                continue;
            }

            candidates++;
            if (rand() % candidates == 0)
            {
                selected.x = x;
                selected.y = y;
            }
        }
    }

    if (candidates == 0)
    {
        return 0;
    }

    game->traps[game->trap_count++] = selected;
    return 1;
}

static void sync_traps(Game *game)
{
    int target = game_trap_target(game->snake.length);

    while (game->trap_count < target)
    {
        if (!place_trap(game))
        {
            break;
        }
    }
}

int game_is_full(const Game *game)
{
    return game->snake.length >= MAX_SNAKE_LENGTH;
}

static int place_food(Game *game)
{
    int free_cells = 0;
    Point selected = {0, 0};

    for (int y = 1; y < BOARD_HEIGHT - 1; y++)
    {
        for (int x = 1; x < BOARD_WIDTH - 1; x++)
        {
            if (!game_is_occupied(game, x, y) && !game_is_trap(game, x, y))
            {
                free_cells++;
                if (rand() % free_cells == 0)
                {
                    selected.x = x;
                    selected.y = y;
                }
            }
        }
    }

    if (free_cells == 0)
    {
        game->has_food = 0;
        return 0;
    }

    game->food = selected;
    game->has_food = 1;
    return 1;
}

int game_init(Game *game, int difficulty)
{
    int center_x = BOARD_WIDTH / 2;
    int center_y = BOARD_HEIGHT / 2;

    memset(game, 0, sizeof(*game));
    game->difficulty = difficulty < 0 || difficulty >= DIFFICULTY_COUNT ? 2 : difficulty;
    game->state = GAME_RUNNING;
    game->snake.length = INITIAL_SNAKE_LENGTH;
    game->snake.direction = DIR_RIGHT;

    for (int i = 0; i < INITIAL_SNAKE_LENGTH; i++)
    {
        game->snake.body[i].x = center_x - i;
        game->snake.body[i].y = center_y;
    }

    if (!place_food(game))
    {
        game->state = GAME_WON;
        return 0;
    }
    return 1;
}

int game_try_set_direction(Game *game, Direction direction)
{
    if (direction < DIR_UP || direction > DIR_LEFT)
    {
        return 0;
    }

    if ((game->snake.direction == DIR_UP && direction == DIR_DOWN) ||
        (game->snake.direction == DIR_DOWN && direction == DIR_UP) ||
        (game->snake.direction == DIR_LEFT && direction == DIR_RIGHT) ||
        (game->snake.direction == DIR_RIGHT && direction == DIR_LEFT))
    {
        return 0;
    }

    game->snake.direction = direction;
    return 1;
}

StepResult game_update(Game *game)
{
    Point next;
    int eating;
    int collision_end;

    if (game->state != GAME_RUNNING)
    {
        return game->state == GAME_WON ? STEP_WON : STEP_LOST;
    }

    next = next_head(game);
    if (next.x <= 0 || next.x >= BOARD_WIDTH - 1 ||
        next.y <= 0 || next.y >= BOARD_HEIGHT - 1)
    {
        game->state = GAME_LOST;
        return STEP_LOST;
    }

    if (game_is_trap(game, next.x, next.y))
    {
        game->state = GAME_LOST;
        return STEP_LOST;
    }

    eating = game->has_food && next.x == game->food.x && next.y == game->food.y;
    collision_end = game->snake.length - (eating ? 0 : 1);

    for (int i = 1; i < collision_end; i++)
    {
        if (game->snake.body[i].x == next.x && game->snake.body[i].y == next.y)
        {
            game->state = GAME_LOST;
            return STEP_LOST;
        }
    }

    if (eating)
    {
        if (game->snake.length >= MAX_SNAKE_LENGTH)
        {
            game->state = GAME_WON;
            return STEP_WON;
        }

        for (int i = game->snake.length; i > 0; i--)
        {
            game->snake.body[i] = game->snake.body[i - 1];
        }
        game->snake.length++;
    }
    else
    {
        for (int i = game->snake.length - 1; i > 0; i--)
        {
            game->snake.body[i] = game->snake.body[i - 1];
        }
    }

    game->snake.body[0] = next;

    if (eating)
    {
        game->score += SCORE_PER_FOOD;
        sync_traps(game);
        if (game_is_full(game) || !place_food(game))
        {
            game->state = GAME_WON;
            return STEP_WON;
        }
    }

    return STEP_CONTINUE;
}

void game_render(const Game *game, int best_score)
{
    char header[160];
    char status[160];

    console_set_color(COLOR_CYAN);
    write_cleared_line(0, "  GREEDY SNAKE  /  贪吃蛇");

    snprintf(header, sizeof(header), "  得分: %04d    最高分: %04d    难度: %s",
             game->score, best_score, difficulty_name(game->difficulty));
    console_set_color(COLOR_YELLOW);
    write_cleared_line(1, header);
    console_set_color(COLOR_DIM);

    console_gotoxy(0, 2);
    putchar('+');
    for (int x = 1; x < BOARD_WIDTH - 1; x++)
    {
        putchar('-');
    }
    putchar('+');

    for (int y = 0; y < BOARD_HEIGHT; y++)
    {
        int current_color = -1;
        console_gotoxy(0, y + 3);
        console_set_color(COLOR_DIM);
        putchar('|');

        for (int x = 0; x < BOARD_WIDTH; x++)
        {
            int desired_color = COLOR_DEFAULT;
            char cell = ' ';

            if (y == 0 || y == BOARD_HEIGHT - 1 || x == 0 || x == BOARD_WIDTH - 1)
            {
                desired_color = COLOR_DIM;
                cell = '=';
            }
            else if (x == game->snake.body[0].x && y == game->snake.body[0].y)
            {
                desired_color = COLOR_BRIGHT_GREEN;
                cell = '@';
            }
            else if (game_is_occupied(game, x, y))
            {
                desired_color = COLOR_GREEN;
                cell = '*';
            }
            else if (game_is_trap(game, x, y))
            {
                desired_color = COLOR_MAGENTA;
                cell = 'X';
            }
            else if (game->has_food && x == game->food.x && y == game->food.y)
            {
                desired_color = COLOR_RED;
                cell = '#';
            }

            if (desired_color != current_color)
            {
                console_set_color((ConsoleColor)desired_color);
                current_color = desired_color;
            }
            putchar(cell);
        }

        console_set_color(COLOR_DIM);
        putchar('|');
    }

    console_gotoxy(0, BOARD_HEIGHT + 3);
    console_set_color(COLOR_DIM);
    putchar('+');
    for (int x = 1; x < BOARD_WIDTH - 1; x++)
    {
        putchar('-');
    }
    putchar('+');

    snprintf(status, sizeof(status), "  长度: %d / %d    陷阱: %d / %d    食物: +%d 分",
             game->snake.length, MAX_SNAKE_LENGTH, game->trap_count, MAX_TRAPS,
             SCORE_PER_FOOD);
    console_set_color(COLOR_DEFAULT);
    write_cleared_line(INFO_ROW, status);
    console_set_color(COLOR_DIM);
    write_cleared_line(CONTROL_ROW, "  WASD / 方向键移动    P 暂停    Q 返回主菜单");
    console_set_color(COLOR_DEFAULT);
    write_cleared_line(STATUS_ROW, "");
}

static void render_pause_state(int paused)
{
    if (paused)
    {
        console_set_color(COLOR_YELLOW);
        write_cleared_line(STATUS_ROW, "  游戏已暂停，按 P 继续，按 Q 退出本局");
    }
    else
    {
        console_set_color(COLOR_DEFAULT);
        write_cleared_line(STATUS_ROW, "");
    }
}

static void show_title_block(const char *title, const char *subtitle)
{
    console_set_color(COLOR_CYAN);
    printf("================================================================\n");
    printf("  %s\n", title);
    console_set_color(COLOR_DIM);
    printf("  %s\n", subtitle);
    console_set_color(COLOR_CYAN);
    printf("================================================================\n");
    console_set_color(COLOR_DEFAULT);
}

static int choose_difficulty(void)
{
    char input[32];
    int choice;

    console_clear();
    show_title_block("选择难度", "速度会随难度提高，得分规则保持一致。");
    printf("\n");
    for (int i = 0; i < DIFFICULTY_COUNT; i++)
    {
        console_set_color(i == 2 ? COLOR_YELLOW : COLOR_DEFAULT);
        printf("    %d. %-6s  %4d ms / 格%s\n",
               i + 1, difficulty_name(i), difficulty_delay(i), i == 2 ? "  [推荐]" : "");
    }
    console_set_color(COLOR_DIM);
    printf("\n    0. 返回主菜单\n");
    console_set_color(COLOR_DEFAULT);
    printf("\n请选择：");

    while (1)
    {
        if (!read_line(input, sizeof(input)))
        {
            return -1;
        }
        if (sscanf(input, "%d", &choice) == 1)
        {
            if (choice >= 1 && choice <= DIFFICULTY_COUNT)
            {
                return choice - 1;
            }
            if (choice == 0)
            {
                return -1;
            }
        }
        console_set_color(COLOR_RED);
        printf("输入无效，请输入 0 - %d：", DIFFICULTY_COUNT);
        console_set_color(COLOR_DEFAULT);
    }
}

static void get_timestamp(char *buffer, size_t capacity)
{
    time_t raw_time = time(NULL);
    struct tm *local = localtime(&raw_time);

    if (local == NULL)
    {
        strncpy(buffer, "0000-00-00 00:00:00", capacity - 1);
        buffer[capacity - 1] = '\0';
        return;
    }

    strftime(buffer, capacity, "%Y-%m-%d %H:%M:%S", local);
}

static void finish_game(const Game *game, RecordList *records)
{
    char input[128];
    char player_name[MAX_NAME_BYTES];
    GameRecord record;
    int save_choice;

    console_clear();
    console_show_cursor();

    if (game->state == GAME_WON)
    {
        console_set_color(COLOR_YELLOW);
        printf("==============================================================\n");
        printf("  恭喜通关！你填满了整个棋盘。\n");
    }
    else
    {
        console_set_color(COLOR_RED);
        printf("==============================================================\n");
        printf("  Game Over\n");
    }

    console_set_color(COLOR_DEFAULT);
    printf("==============================================================\n");
    printf("  本局得分：%d\n", game->score);
    printf("  最终长度：%d\n", game->snake.length);
    printf("  场上陷阱：%d\n", game->trap_count);
    printf("  难度等级：%s\n", difficulty_name(game->difficulty));
    printf("  历史最高：%d\n", records_best_score(records));
    printf("\n是否保存本局记录？(y/n) ");

    console_flush_input();
    if (!read_line(input, sizeof(input)))
    {
        return;
    }
    save_choice = tolower((unsigned char)input[0]);
    if (save_choice != 'y')
    {
        return;
    }

    console_set_color(COLOR_DIM);
    printf("\n请输入玩家名（留空则使用 Player）：");
    console_set_color(COLOR_DEFAULT);
    if (!read_line(input, sizeof(input)))
    {
        return;
    }

    records_sanitize_name(player_name, sizeof(player_name), input);
    memset(&record, 0, sizeof(record));
    snprintf(record.name, sizeof(record.name), "%s", player_name);
    record.score = game->score;
    record.difficulty = game->difficulty;
    get_timestamp(record.time, sizeof(record.time));

    if (records_append(&record) == 0)
    {
        if (records_load(records) != 0)
        {
            console_set_color(COLOR_YELLOW);
            printf("\n记录已写入，但排行榜重新加载失败。\n");
        }
        else
        {
            console_set_color(COLOR_GREEN);
            printf("\n记录保存成功。\n");
        }
    }
    else
    {
        console_set_color(COLOR_RED);
        printf("\n记录保存失败，请检查当前目录的写入权限。\n");
    }

    console_set_color(COLOR_DIM);
    printf("\n按 Enter 返回主菜单...");
    console_set_color(COLOR_DEFAULT);
    (void)read_line(input, sizeof(input));
}

static Direction arrow_direction(int scan_code)
{
    switch (scan_code)
    {
    case 72:
        return DIR_UP;
    case 80:
        return DIR_DOWN;
    case 75:
        return DIR_LEFT;
    case 77:
        return DIR_RIGHT;
    default:
        return DIR_NONE;
    }
}

static void play_game(RecordList *records)
{
    Game game;
    Direction queued_direction = DIR_NONE;
    int direction_queued = 0;
    int paused = 0;
    int quit_to_menu = 0;
    int finished = 0;
    int difficulty;
    ULONGLONG last_step;

    difficulty = choose_difficulty();
    if (difficulty < 0)
    {
        return;
    }

    if (!game_init(&game, difficulty))
    {
        console_clear();
        printf("无法初始化棋盘。\n");
        wait_for_enter();
        return;
    }

    console_clear();
    console_hide_cursor();
    game_render(&game, records_best_score(records));
    last_step = GetTickCount64();

    while (!finished)
    {
        while (_kbhit())
        {
            int key = _getch();
            Direction direction = DIR_NONE;

            if (key == 0 || key == 224)
            {
                direction = arrow_direction(_getch());
                key = -1;
            }
            else if (key >= 0 && key < 256)
            {
                key = tolower((unsigned char)key);
            }

            if (direction != DIR_NONE)
            {
                if (!paused && !direction_queued)
                {
                    queued_direction = direction;
                    direction_queued = 1;
                }
                continue;
            }

            if (key == 'p' || key == ' ' || key == 27)
            {
                paused = !paused;
                if (paused)
                {
                    direction_queued = 0;
                    queued_direction = DIR_NONE;
                }
                else
                {
                    last_step = GetTickCount64();
                    game_render(&game, records_best_score(records));
                }
                render_pause_state(paused);
            }
            else if (key == 'q')
            {
                quit_to_menu = 1;
                finished = 1;
            }
            else if (!paused && !direction_queued)
            {
                switch (key)
                {
                case 'w':
                    queued_direction = DIR_UP;
                    direction_queued = 1;
                    break;
                case 'a':
                    queued_direction = DIR_LEFT;
                    direction_queued = 1;
                    break;
                case 's':
                    queued_direction = DIR_DOWN;
                    direction_queued = 1;
                    break;
                case 'd':
                    queued_direction = DIR_RIGHT;
                    direction_queued = 1;
                    break;
                default:
                    break;
                }
            }
        }

        if (finished)
        {
            break;
        }

        if (!paused && GetTickCount64() - last_step >= (ULONGLONG)difficulty_delay(game.difficulty))
        {
            if (direction_queued)
            {
                (void)game_try_set_direction(&game, queued_direction);
                direction_queued = 0;
            }

            if (game_update(&game) != STEP_CONTINUE)
            {
                finished = 1;
            }
            game_render(&game, records_best_score(records));
            last_step = GetTickCount64();
        }

        Sleep(10);
    }

    console_show_cursor();
    if (!quit_to_menu)
    {
        finish_game(&game, records);
    }
}

static void show_main_menu(const RecordList *records)
{
    console_clear();
    show_title_block("贪吃蛇控制台版", "吃下食物、避开墙壁、身体和随机陷阱。");
    console_set_color(COLOR_YELLOW);
    printf("  历史最高分：%d\n", records_best_score(records));
    console_set_color(COLOR_DEFAULT);
    printf("\n");
    printf("  1. 开始游戏\n");
    printf("  2. 查看排行榜\n");
    printf("  3. 查看历史记录\n");
    printf("  4. 游戏规则\n");
    printf("  5. 退出游戏\n");
    console_set_color(COLOR_DIM);
    printf("\n  提示：游戏内按 P 暂停，按 Q 立即返回主菜单。\n");
    console_set_color(COLOR_DEFAULT);
    printf("\n请选择：");
}

static void show_rules(void)
{
    console_clear();
    show_title_block("游戏规则", "控制蛇持续移动，尽可能获得更高的分数。");
    printf("\n");
    printf("  1. 使用 W A S D 或方向键控制移动方向。\n");
    printf("  2. 每吃到一个 # 增加 1 节身体，并获得 10 分。\n");
    printf("  3. 撞到边界 = 或自己的身体 * 会失败，反向输入会被忽略。\n");
    printf("  4. 蛇长达到 10 后出现陷阱 X，之后每增加 3 节多 1 个。\n");
    printf("  5. 陷阱会一直保留，数量最多 12 个，撞到即游戏结束。\n");
    printf("  6. 食物和陷阱不会生成在蛇身上，新陷阱也会远离蛇头。\n");
    printf("  7. 游戏内按 P 暂停，按 Q 返回主菜单（本局不保存）。\n");
    printf("  8. 通关后排行榜按分数、难度和时间依次排序。\n");
    wait_for_enter();
}

int main(int argc, char **argv)
{
    RecordList records;

    if (argc > 1 && strcmp(argv[1], "--self-test") == 0)
    {
        return run_self_tests();
    }

    srand((unsigned int)time(NULL) ^ GetCurrentProcessId());
    console_init();

    if (records_load(&records) != 0)
    {
        console_clear();
        console_set_color(COLOR_RED);
        printf("无法读取 record.txt，请检查文件权限。\n");
        console_set_color(COLOR_DEFAULT);
        wait_for_enter();
    }

    while (1)
    {
        char input[32];
        int choice;

        show_main_menu(&records);
        if (!read_line(input, sizeof(input)) || sscanf(input, "%d", &choice) != 1)
        {
            continue;
        }

        switch (choice)
        {
        case 1:
            play_game(&records);
            break;
        case 2:
            records_show_rank(&records);
            wait_for_enter();
            break;
        case 3:
            records_show_all(&records);
            wait_for_enter();
            break;
        case 4:
            show_rules();
            break;
        case 5:
            console_clear();
            console_set_color(COLOR_CYAN);
            printf("感谢游玩，再见！\n");
            console_restore();
            return 0;
        default:
            break;
        }
    }
}

static int expect(int condition, const char *name, int *passed, int *failed)
{
    if (condition)
    {
        (*passed)++;
        printf("[PASS] %s\n", name);
        return 1;
    }

    (*failed)++;
    printf("[FAIL] %s\n", name);
    return 0;
}

int run_self_tests(void)
{
    Game game;
    StepResult result;
    int passed = 0;
    int failed = 0;
    int food_safe = 1;

    srand(20261019);

    expect(game_trap_target(9) == 0, "no traps below the threshold", &passed, &failed);
    expect(game_trap_target(10) == 1, "first trap appears at length 10", &passed, &failed);
    expect(game_trap_target(13) == 2, "trap count grows every three cells", &passed, &failed);
    expect(game_trap_target(1000) == MAX_TRAPS, "trap count is capped", &passed, &failed);

    expect(game_init(&game, 2) == 1, "game initializes", &passed, &failed);
    expect(game.snake.length == INITIAL_SNAKE_LENGTH, "initial snake length is 3", &passed, &failed);
    for (int i = 0; i < game.snake.length; i++)
    {
        if (game.food.x == game.snake.body[i].x && game.food.y == game.snake.body[i].y)
        {
            food_safe = 0;
        }
    }
    expect(food_safe, "initial food does not overlap the snake", &passed, &failed);
    expect(game_try_set_direction(&game, DIR_LEFT) == 0, "direct reversal is rejected", &passed, &failed);
    expect(game_try_set_direction(&game, DIR_UP) == 1, "perpendicular turn is accepted", &passed, &failed);

    game_init(&game, 2);
    game.snake.body[0].x = 1;
    game.snake.body[0].y = 5;
    game.snake.body[1].x = 2;
    game.snake.body[1].y = 5;
    game.snake.body[2].x = 3;
    game.snake.body[2].y = 5;
    game.snake.direction = DIR_LEFT;
    result = game_update(&game);
    expect(result == STEP_LOST && game.state == GAME_LOST, "wall collision ends the game", &passed, &failed);

    game_init(&game, 2);
    game.snake.length = 4;
    game.snake.direction = DIR_DOWN;
    game.snake.body[0] = (Point){2, 1};
    game.snake.body[1] = (Point){1, 1};
    game.snake.body[2] = (Point){1, 2};
    game.snake.body[3] = (Point){2, 2};
    game.food = (Point){10, 10};
    result = game_update(&game);
    expect(result == STEP_CONTINUE && game.snake.length == 4,
           "moving into a vacated tail cell is allowed", &passed, &failed);

    game_init(&game, 2);
    game.snake.length = 7;
    game.snake.direction = DIR_UP;
    game.snake.body[0] = (Point){3, 2};
    game.snake.body[1] = (Point){2, 2};
    game.snake.body[2] = (Point){1, 2};
    game.snake.body[3] = (Point){1, 1};
    game.snake.body[4] = (Point){2, 1};
    game.snake.body[5] = (Point){3, 1};
    game.snake.body[6] = (Point){4, 1};
    game.food = (Point){10, 10};
    result = game_update(&game);
    expect(result == STEP_LOST, "body collision ends the game", &passed, &failed);

    game_init(&game, 2);
    game.food.x = game.snake.body[0].x + 1;
    game.food.y = game.snake.body[0].y;
    result = game_update(&game);
    expect(result == STEP_CONTINUE && game.snake.length == 4 && game.score == SCORE_PER_FOOD,
           "eating food grows the snake and adds score", &passed, &failed);
    food_safe = 1;
    for (int i = 0; i < game.snake.length; i++)
    {
        if (game.food.x == game.snake.body[i].x && game.food.y == game.snake.body[i].y)
        {
            food_safe = 0;
        }
    }
    expect(food_safe, "replacement food is placed on a free cell", &passed, &failed);

    game_init(&game, 2);
    game.trap_count = 1;
    game.traps[0] = (Point){game.snake.body[0].x + 1, game.snake.body[0].y};
    game.food = (Point){1, 1};
    result = game_update(&game);
    expect(result == STEP_LOST, "trap collision ends the game", &passed, &failed);

    game_init(&game, 2);
    game.snake.length = 9;
    game.snake.direction = DIR_RIGHT;
    for (int i = 0; i < game.snake.length; i++)
    {
        game.snake.body[i] = (Point){20 - i, 10};
    }
    game.food = (Point){21, 10};
    result = game_update(&game);
    expect(result == STEP_CONTINUE && game.snake.length == 10 && game.trap_count == 1,
           "growing to length 10 creates a trap", &passed, &failed);
    expect(game.trap_count == 1 && !game_is_occupied(&game, game.traps[0].x, game.traps[0].y),
           "new trap does not overlap the snake", &passed, &failed);
    expect(game.trap_count == 1 &&
               !(game.food.x == game.traps[0].x && game.food.y == game.traps[0].y) &&
               abs(game.traps[0].x - game.snake.body[0].x) +
                       abs(game.traps[0].y - game.snake.body[0].y) >=
                   3,
           "new trap stays away from the head and food", &passed, &failed);

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
