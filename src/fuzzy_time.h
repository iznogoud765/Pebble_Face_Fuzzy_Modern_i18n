#pragma once
#include "time.h"

#define DEBUG 0 // 0=nodebug, 1=test, 2=german, 3=spanish, 4=french

int fuzzy_time(struct tm* t, char* str_line1, char* str_line2, char* str_line3);

void info_lines(struct tm* t, char* str_line1, char* str_line2);