#ifndef USER_H
#define USER_H

#include <stddef.h>
#include "snake.h"

int records_load(RecordList *records);
int records_append(const GameRecord *record);
int records_best_score(const RecordList *records);
void records_sanitize_name(char *destination, size_t capacity, const char *source);
void records_show_all(const RecordList *records);
void records_show_rank(const RecordList *records);

#endif
