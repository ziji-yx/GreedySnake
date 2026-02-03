#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#define LOW 20
#define COL 50
#include "snake.h"
#include "user.h"
#include "win.h"

int main(void)
{
    // 初始化随机数种子
    srand((unsigned int)time(NULL));

    Hallo();
    while (1)
    {
        int choice;
        scanf("%d", &choice);
        while (getchar() != '\n')
            ;
        switch (choice)
        {
        case 1:
            SetSpeed();
            start();
            break;
        case 2:
            if (GetRecordFromFile() == 1)
                printf("程序异常，读取失败！\n");
            else
                PrintNode();
            break;
        case 3:
            if (GetRecordFromFile() == 1)
                printf("程序异常，读取失败！\n");
            else
                CheckRank();
            break;
        case 4:
            Rule();
            break;
        case 5:
            return 0;
        }
    }
}

void start()
{
    checkdeath = 0;
    score = 0;
    InitMap();
    InitSnake();
    CreateFood();
    PrintMap();
    system("cls");
    HideCursor();
    while (1)
    {
        gotoxy(0, 0);
        ChangeDirectionFromKeybroad();
        MoveSnake();
        InitMap();
        if (checkdeath)
        {
            Lose();
            Hallo();
            return;
        }
        CheckFood();
        UpdateToMap();
        PrintMap();
    }
}

void InitMap()
{
    for (int i = 0; i < LOW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            map[i][j] = ' ';
        }
    }

    for (int i = 0; i < LOW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            if (i == 0 || j == 0 || i == LOW - 1 || j == COL - 1)
            {
                map[i][j] = '=';
            }
        }
    }
}

void PrintMap()
{
    for (int i = 0; i < LOW; i++)
    {
        for (int j = 0; j < COL; j++)
        {
            printf("%c", map[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    printf("score:%d\n", score);
}

void InitSnake()
{
    snake->length = 3;
    snake->body[2].x = 1;
    snake->body[2].y = 1;
    snake->body[1].x = 2;
    snake->body[1].y = 1;
    snake->body[0].x = 3;
    snake->body[0].y = 1;
    snake->direction = 'D';
}

void UpdateToMap()
{
    for (int i = 1; i < snake->length; i++)
    {
        map[snake->body[i].y][snake->body[i].x] = '*';
    }
    map[snake->body[0].y][snake->body[0].x] = '@';
    map[food.y][food.x] = '#';
}

void MoveSnake()
{
    for (int i = snake->length - 1; i > 0; i--)
    {
        snake->body[i].x = snake->body[i - 1].x;
        snake->body[i].y = snake->body[i - 1].y;
    }
    switch (snake->direction)
    {
    case 'W':

        snake->body[0].y--;
        break;
    case 'A':

        snake->body[0].x--;
        break;
    case 'S':

        snake->body[0].y++;
        break;
    case 'D':

        snake->body[0].x++;
        break;
    }
    Sleep(speed);
    if (map[snake->body[0].y][snake->body[0].x] != ' ' && map[snake->body[0].y][snake->body[0].x] != '#')
    {
        checkdeath = 1;
    }
}

void gotoxy(int x, int y)
{
    COORD pos = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void ChangeDirectionFromKeybroad()
{
    if (kbhit())
    {
        char input = getch();
        switch (input)
        {
        case 'W':
        case 'w':
            if (snake->direction != 'S')
            {
                snake->direction = 'W';
            }
            break;
        case 'A':
        case 'a':
            if (snake->direction != 'D')
            {
                snake->direction = 'A';
            }
            break;
        case 'S':
        case 's':
            if (snake->direction != 'W')
            {
                snake->direction = 'S';
            }
            break;
        case 'D':
        case 'd':
            if (snake->direction != 'A')
            {
                snake->direction = 'D';
            }
            break;
        }
    }
}

void HideCursor()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor_info;
    GetConsoleCursorInfo(hOut, &cursor_info);
    cursor_info.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursor_info);
}

void ExposeCursor()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor_info;
    GetConsoleCursorInfo(hOut, &cursor_info);
    cursor_info.bVisible = TRUE;
    SetConsoleCursorInfo(hOut, &cursor_info);
}

void CreateFood()
{
    do
    {
        food.x = rand() % (COL - 2) + 1;
        food.y = rand() % (LOW - 2) + 1;
    } while (map[food.y][food.x] != ' ');
}

void Hallo()
{
    printf("==========================\n");
    printf("欢迎游玩贪吃蛇小游戏！\n");
    printf("请选择：\n");
    printf("1.开始游戏\n");
    printf("2.查看记录\n");
    printf("3.查看排名\n");
    printf("4.游戏规则\n");
    printf("5.退出游戏\n");
    printf("==========================\n");
}

void SetSpeed()
{
    printf("===================\n");
    printf("请选择你的速度：\n");
    printf("1.鼻涕虫\n");
    printf("2.蠕虫\n");
    printf("3.菜蛇\n");
    printf("4.蟒蛇\n");
    printf("5.耶梦加得\n");
    printf("===================\n");
    int choice;
    scanf("%d", &choice);
    switch (choice)
    {
    case 1:
        speed = 200;
        break;
    case 2:
        speed = 100;
        break;
    case 3:
        speed = 50;
        break;
    case 4:
        speed = 25;
        break;
    case 5:
        speed = 10;
        break;
    }
}

void CheckFood()
{
    if (snake->body[0].x == food.x && snake->body[0].y == food.y)
    {
        snake->length++;
        score += 10;
        CreateFood();
    }
}

void Lose()
{
    system("cls");
    printf("==========================\n");
    printf("       Game Over!         \n");
    printf("       最终得分: %d        \n", score);
    printf("==========================\n");
    printf("\n");
    printf("是否保存此记录 y/n\n");
    ExposeCursor();
    char choice;
    while (getchar() != '\n')
        ;
    scanf("%c", &choice);
    if (choice == 'Y' || choice == 'y')
    {
        if (Record() == 1)
        {
            printf("程序异常,保存失败！");
        }
        else
        {
            printf("保存成功！");
        }
    }
    printf("按任意键返回主菜单...\n");
    getch();
    system("cls");
}

void Rule()
{
    system("cls");
    printf("游戏规则：\n");
    printf("1. 使用 W A S D 控制蛇的方向。\n");
    printf("2. 吃到 # 增加长度并获得 10 分。\n");
    printf("3. 撞到墙壁 = 或自己的身体 * 则游戏结束。\n");
    printf("\n按任意键返回...");
    getch();
    system("cls");
    Hallo();
}

int Record()
{
    FILE *fp;
    fp = fopen("record.txt", "a+");
    if (fp == NULL)
        return 1;
    char name[30];
    char level_name[20];
    if (speed == 200)
        strcpy(level_name, "鼻涕虫");
    else if (speed == 100)
        strcpy(level_name, "蠕虫");
    else if (speed == 50)
        strcpy(level_name, "菜蛇");
    else if (speed == 25)
        strcpy(level_name, "蟒蛇");
    else if (speed == 10)
        strcpy(level_name, "耶梦加得");
    else
        strcpy(level_name, "未知");
    printf("请输入你的用户名：");
    scanf("%s", name);
    GetFinishTime(timestr);
    fprintf(fp, " name:%s | score: %d | speed: %s | %s\n", name, score, level_name, timestr);
    fclose(fp);
    for (int i = 0; timestr[i] != '\0'; i++)
    {
        timestr[i] = 0;
    }
    return 0;
}

int GetRecordFromFile()
{
    FreeNode();
    FILE *fp;
    fp = fopen("record.txt", "r");
    if (fp == NULL)
        return 1;
    char name[30], time1[20], time2[20], s_level[20];
    int sco;
    while (fscanf(fp, " name:%s | score: %d | speed: %s | %s %s\n", name, &sco, s_level, time1, time2) == 5)
    {
        data_node *new = (data_node *)malloc(sizeof(data_node));
        if (new == NULL)
            break;
        if (head == NULL)
        {
            head = new;
            tail = new;
            new->num = 1;
        }
        else
        {
            new->num = tail->num + 1;
            tail->next = new;
            tail = new;
        }
        strcpy(new->name, name);
        strcpy(new->speed_level, s_level);
        sprintf(new->time, "%s %s", time1, time2);
        new->score = sco;
        new->next = NULL;
    }
    fclose(fp);
    return 0;
}

void GetFinishTime(char *timestr)
{
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(timestr, 20, "%Y-%m-%d %H:%M:%S", info);
}

void FreeNode()
{
    if (head != NULL)
    {
        data_node *cur = head->next;
        head->next = NULL;
        while (cur != NULL)
        {
            data_node *tmp = cur;
            cur = cur->next;
            free(tmp);
        }
    }
    head = NULL;
    tail = NULL;
}

void PrintNode()
{
    if (head == NULL)
    {
        printf("暂无记录！\n");
        return;
    }
    data_node *cur = head;
    printf("========================== 历史记录（按时间）=============================\n");
    printf(" %-6s | %-12s | %-6s | %-10s | %-20s \n", "排名", "用户名", "分数", "难度等级", "结束时间");
    printf("--------------------------------------------------------------------------\n");
    while (cur != NULL)
    {
        printf(" %-5d  |", cur->num);
        printf(" %-12s |", cur->name);
        printf(" %-6d |", cur->score);
        printf(" %-10s |", cur->speed_level);
        printf(" %-19s\n", cur->time);
        cur = cur->next;
    }
    printf("==========================================================================\n");
    printf("\n按任意键返回...");
    getch();
    system("cls");
    Hallo();
}

void CheckRank()
{
    if (head == NULL)
    {
        printf("暂无记录，无法排名！\n");
        return;
    }

    data_node *i, *j;
    int tempScore, tempNum;
    char tempName[30], tempLevel[20], tempTime[20];

    for (i = head; i != NULL; i = i->next)
    {
        for (j = i->next; j != NULL; j = j->next)
        {
            if (i->score < j->score)
            {
                tempScore = i->score;
                i->score = j->score;
                j->score = tempScore;
                strcpy(tempName, i->name);
                strcpy(i->name, j->name);
                strcpy(j->name, tempName);
                strcpy(tempLevel, i->speed_level);
                strcpy(i->speed_level, j->speed_level);
                strcpy(j->speed_level, tempLevel);
                strcpy(tempTime, i->time);
                strcpy(i->time, j->time);
                strcpy(j->time, tempTime);
            }
        }
    }

    data_node *cur = head;
    printf("========================== 历史记录（按分数）=============================\n");
    printf(" %-6s | %-12s | %-6s | %-10s | %-20s \n", "排名", "用户名", "分数", "难度等级", "结束时间");
    printf("--------------------------------------------------------------------------\n");
    while (cur != NULL)
    {
        printf(" %-5d  |", cur->num);
        printf(" %-12s |", cur->name);
        printf(" %-6d |", cur->score);
        printf(" %-10s |", cur->speed_level);
        printf(" %-19s\n", cur->time);
        cur = cur->next;
    }
    printf("==========================================================================\n");
    printf("\n按任意键返回...");
    getch();
    system("cls");
    Hallo();
}
