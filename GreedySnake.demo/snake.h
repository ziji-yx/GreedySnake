typedef struct
{
    int x;
    int y;
} Point;
typedef struct
{
    Point body[100];
    char direction;
    int length;
} Snake;
Point food;
Snake v;
Snake *snake = &v;
int score;
char map[LOW][COL];
int checkdeath;
void start();
void InitMap();
void PrintMap();
void InitSnake();
void UpdateToMap();
void MoveSnake();
void CreateFood();
void CheckFood();
void CheckClash();
void Lose();