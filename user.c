#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"
#include "win.h"

#define RECORD_FILE "record.txt"
#define LEADERBOARD_SIZE 10
#define RECENT_RECORD_SIZE 15

static void trim_whitespace(char *text)
{
    char *start = text;
    char *end;

    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n')
    {
        start++;
    }

    if (start != text)
    {
        memmove(text, start, strlen(start) + 1);
    }

    end = text + strlen(text);
    while (end > text && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n'))
    {
        end--;
    }
    *end = '\0';
}

static void add_record(RecordList *records, const GameRecord *record)
{
    if (records->count >= MAX_RECORDS)
    {
        return;
    }

    records->items[records->count] = *record;
    records->items[records->count].name[MAX_NAME_BYTES - 1] = '\0';
    records->items[records->count].time[MAX_TIME_BYTES - 1] = '\0';
    records->count++;
}

static int parse_new_record(char *line, GameRecord *record)
{
    char name[MAX_NAME_BYTES] = {0};
    char timestamp[MAX_TIME_BYTES] = {0};
    int score;
    int difficulty;

    if (sscanf(line, "%39[^|]|%d|%d|%23[^\r\n]", name, &score, &difficulty, timestamp) != 4)
    {
        return 0;
    }

    trim_whitespace(name);
    trim_whitespace(timestamp);
    if (name[0] == '\0' || timestamp[0] == '\0' || score < 0)
    {
        return 0;
    }

    memset(record, 0, sizeof(*record));
    snprintf(record->name, sizeof(record->name), "%s", name);
    snprintf(record->time, sizeof(record->time), "%s", timestamp);
    record->score = score;
    record->difficulty = difficulty < 0 || difficulty >= DIFFICULTY_COUNT ? 2 : difficulty;
    return 1;
}

static int parse_legacy_record(char *line, GameRecord *record)
{
    char name[MAX_NAME_BYTES] = {0};
    char difficulty_text[MAX_NAME_BYTES] = {0};
    char timestamp[MAX_TIME_BYTES] = {0};
    int score;

    if (sscanf(line, " name:%39[^|] | score: %d | speed: %39[^|] | %23[^\r\n]",
               name, &score, difficulty_text, timestamp) != 4)
    {
        return 0;
    }

    trim_whitespace(name);
    trim_whitespace(timestamp);
    if (name[0] == '\0' || timestamp[0] == '\0' || score < 0)
    {
        return 0;
    }

    memset(record, 0, sizeof(*record));
    snprintf(record->name, sizeof(record->name), "%s", name);
    snprintf(record->time, sizeof(record->time), "%s", timestamp);
    record->score = score;
    record->difficulty = 2;

    if (strstr(difficulty_text, "悠闲") != NULL)
    {
        record->difficulty = 0;
    }
    else if (strstr(difficulty_text, "轻松") != NULL)
    {
        record->difficulty = 1;
    }
    else if (strstr(difficulty_text, "标准") != NULL)
    {
        record->difficulty = 2;
    }
    else if (strstr(difficulty_text, "挑战") != NULL)
    {
        record->difficulty = 3;
    }
    else if (strstr(difficulty_text, "极限") != NULL)
    {
        record->difficulty = 4;
    }

    return 1;
}

int records_load(RecordList *records)
{
    FILE *file;
    char line[512];

    records->count = 0;
    file = fopen(RECORD_FILE, "r");
    if (file == NULL)
    {
        return errno == ENOENT ? 0 : 1;
    }

    while (fgets(line, sizeof(line), file) != NULL)
    {
        GameRecord record;
        if (parse_new_record(line, &record) || parse_legacy_record(line, &record))
        {
            add_record(records, &record);
        }
    }

    fclose(file);
    return 0;
}

int records_append(const GameRecord *record)
{
    FILE *file = fopen(RECORD_FILE, "a");
    if (file == NULL)
    {
        return 1;
    }

    fprintf(file, "%s|%d|%d|%s\n", record->name, record->score, record->difficulty, record->time);
    if (ferror(file))
    {
        fclose(file);
        return 1;
    }

    fclose(file);
    return 0;
}

int records_best_score(const RecordList *records)
{
    int best = 0;

    for (int i = 0; i < records->count; i++)
    {
        if (records->items[i].score > best)
        {
            best = records->items[i].score;
        }
    }
    return best;
}

void records_sanitize_name(char *destination, size_t capacity, const char *source)
{
    size_t source_index = 0;
    size_t destination_index = 0;

    if (capacity == 0)
    {
        return;
    }

    while (source[source_index] == ' ' || source[source_index] == '\t')
    {
        source_index++;
    }

    while (source[source_index] != '\0' && destination_index + 1 < capacity)
    {
        unsigned char current = (unsigned char)source[source_index];
        size_t character_bytes = 1;

        if ((current & 0xE0) == 0xC0)
        {
            character_bytes = 2;
        }
        else if ((current & 0xF0) == 0xE0)
        {
            character_bytes = 3;
        }
        else if ((current & 0xF8) == 0xF0)
        {
            character_bytes = 4;
        }

        if (destination_index + character_bytes + 1 > capacity)
        {
            break;
        }

        if (character_bytes == 1 && (current == '|' || current == '\r' || current == '\n' || current == '\t'))
        {
            destination[destination_index++] = '_';
            source_index++;
            continue;
        }

        for (size_t i = 0; i < character_bytes; i++)
        {
            destination[destination_index++] = source[source_index++];
        }
    }

    while (destination_index > 0 &&
           (destination[destination_index - 1] == ' ' || destination[destination_index - 1] == '\t'))
    {
        destination_index--;
    }

    destination[destination_index] = '\0';
    if (destination[0] == '\0')
    {
        strncpy(destination, "Player", capacity - 1);
        destination[capacity - 1] = '\0';
    }
}

static int compare_rank(const void *left, const void *right)
{
    const GameRecord *a = (const GameRecord *)left;
    const GameRecord *b = (const GameRecord *)right;

    if (a->score != b->score)
    {
        return b->score - a->score;
    }
    if (a->difficulty != b->difficulty)
    {
        return b->difficulty - a->difficulty;
    }
    return strcmp(b->time, a->time);
}

static int codepoint_width(unsigned int codepoint)
{
    if ((codepoint >= 0x0300 && codepoint <= 0x036F) ||
        (codepoint >= 0x1AB0 && codepoint <= 0x1AFF) ||
        (codepoint >= 0xFE00 && codepoint <= 0xFE0F))
    {
        return 0;
    }

    if ((codepoint >= 0x1100 && codepoint <= 0x115F) ||
        (codepoint >= 0x2E80 && codepoint <= 0xA4CF) ||
        (codepoint >= 0xAC00 && codepoint <= 0xD7A3) ||
        (codepoint >= 0xF900 && codepoint <= 0xFAFF) ||
        (codepoint >= 0xFE10 && codepoint <= 0xFE6F) ||
        (codepoint >= 0xFF00 && codepoint <= 0xFF60) ||
        (codepoint >= 0xFFE0 && codepoint <= 0xFFE6) ||
        (codepoint >= 0x20000 && codepoint <= 0x3FFFD))
    {
        return 2;
    }

    return 1;
}

static int decode_utf8(const unsigned char *text, unsigned int *codepoint)
{
    if (text[0] < 0x80)
    {
        *codepoint = text[0];
        return 1;
    }
    if ((text[0] & 0xE0) == 0xC0 && (text[1] & 0xC0) == 0x80)
    {
        *codepoint = ((unsigned int)(text[0] & 0x1F) << 6) | (text[1] & 0x3F);
        return 2;
    }
    if ((text[0] & 0xF0) == 0xE0 && (text[1] & 0xC0) == 0x80 && (text[2] & 0xC0) == 0x80)
    {
        *codepoint = ((unsigned int)(text[0] & 0x0F) << 12) |
                     ((unsigned int)(text[1] & 0x3F) << 6) |
                     (text[2] & 0x3F);
        return 3;
    }
    if ((text[0] & 0xF8) == 0xF0 && (text[1] & 0xC0) == 0x80 &&
        (text[2] & 0xC0) == 0x80 && (text[3] & 0xC0) == 0x80)
    {
        *codepoint = ((unsigned int)(text[0] & 0x07) << 18) |
                     ((unsigned int)(text[1] & 0x3F) << 12) |
                     ((unsigned int)(text[2] & 0x3F) << 6) |
                     (text[3] & 0x3F);
        return 4;
    }

    *codepoint = '?';
    return 1;
}

static void print_field(const char *text, int field_width)
{
    const unsigned char *current = (const unsigned char *)text;
    int used = 0;

    while (*current != '\0')
    {
        unsigned int codepoint;
        int bytes = decode_utf8(current, &codepoint);
        int width = codepoint_width(codepoint);

        if (used + width > field_width)
        {
            break;
        }

        fwrite(current, 1, (size_t)bytes, stdout);
        used += width;
        current += bytes;
    }

    while (used < field_width)
    {
        putchar(' ');
        used++;
    }
}

static void print_table_header(const char *title)
{
    console_clear();
    console_set_color(COLOR_CYAN);
    printf("==============================================================\n");
    printf("  %s\n", title);
    printf("==============================================================\n");
    console_set_color(COLOR_DEFAULT);
    printf("  %-3s %-14s %6s  %-10s %s\n", "排名", "玩家", "得分", "难度", "时间");
    printf("--------------------------------------------------------------\n");
}

void records_show_rank(const RecordList *records)
{
    GameRecord sorted[MAX_RECORDS];
    int limit;

    print_table_header("排行榜（按得分排序）");
    if (records->count == 0)
    {
        console_set_color(COLOR_DIM);
        printf("  暂无成绩记录，先完成一局吧。\n");
        console_set_color(COLOR_DEFAULT);
    }
    else
    {
        memcpy(sorted, records->items, sizeof(GameRecord) * (size_t)records->count);
        qsort(sorted, (size_t)records->count, sizeof(GameRecord), compare_rank);
        limit = records->count < LEADERBOARD_SIZE ? records->count : LEADERBOARD_SIZE;

        for (int i = 0; i < limit; i++)
        {
            console_set_color(i == 0 ? COLOR_YELLOW : COLOR_DEFAULT);
            printf("  %2d. ", i + 1);
            print_field(sorted[i].name, 14);
            printf(" %5d  ", sorted[i].score);
            print_field(difficulty_name(sorted[i].difficulty), 10);
            printf(" %s\n", sorted[i].time);
        }
    }
    console_set_color(COLOR_DEFAULT);
    printf("--------------------------------------------------------------\n");
}

void records_show_all(const RecordList *records)
{
    int start;
    int limit;

    print_table_header("历史记录（最近 15 局）");
    if (records->count == 0)
    {
        console_set_color(COLOR_DIM);
        printf("  暂无历史记录。\n");
        console_set_color(COLOR_DEFAULT);
    }
    else
    {
        start = records->count > RECENT_RECORD_SIZE ? records->count - RECENT_RECORD_SIZE : 0;
        limit = records->count - start;
        for (int i = 0; i < limit; i++)
        {
            const GameRecord *record = &records->items[start + limit - 1 - i];
            printf("  %2d. ", i + 1);
            print_field(record->name, 14);
            printf(" %5d  ", record->score);
            print_field(difficulty_name(record->difficulty), 10);
            printf(" %s\n", record->time);
        }
    }
    console_set_color(COLOR_DEFAULT);
    printf("--------------------------------------------------------------\n");
}
