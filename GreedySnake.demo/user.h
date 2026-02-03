int speed;
char timestr[30];
typedef struct node
{
    struct node *next;
    char time[30];
    char name[30];
    char speed_level[20];
    int score;
    int num;
} data_node;

data_node *head = NULL;
data_node *tail = NULL;

void Hallo();
void SetSpeed();
void Rule();
int Record();
int GetRecordFromFile();
void GetFinishTime(char *timeStr);
void FreeNode();
void PrintNode();
void CheckRank();
int GetDisplayWidth(const char *str);
void PrintFormattedRow(int rank, char *name, int score, char *level, char *time);