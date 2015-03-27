#include <pebble.h>
#include "time_en.h"
#include "string.h"

const char* const HOURS_EN[] = {
  "twelve",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "ten",
  "eleven",
  "twelve",
};

const char* const REL_EN[] = {
  "it's",
  "five",
  "ten",
  "quarter",
  "twenty",
  "twenty",
  "half"
};

static const char* const ONETEENS[] = {
  "zero",
  "one",
  "two",
  "three",
  "four",
  "five",
  "six",
  "seven",
  "eight",
  "nine",
  "ten",
  "eleven",
  "twelve",
  "thirteen",
  "fourteen",
  "fifteen",
  "sixteen",
  "seventeen",
  "eighteen",
  "nineteen"
};

static const char* const TWENS[] = {
  "twenty",
  "thirty",
  "forty",
  "fifty",
};

static const char* STR_TEEN = "teen";
static const char* STR_OH_CLOCK = "o'clock";
static const char* STR_OH = "o'";
static const char* STR_NOON = "noon";
static const char* STR_MID = "mid-";  // midnight
static const char* STR_NIGHT = "night";  // midnight
static const char* STR_MIDNIGHT = "midnite";  // midnight
static const char* STR_QUARTER = "quarter";
static const char* STR_TO = "to";
static const char* STR_PAST = "past";
static const char* STR_HALF = "half";
static const char* STR_ITS = "it's";
static const char* STR_NOW = "now";
static const char* STR_ALMOST = "almost";
static const char* STR_JUST = "just";
static const char* STR_ALREADY = "already";


int fuzzy_time_en(struct tm* t, char* line1, char* line2, char* line3)
{
  int ret = 3;
  
  int hours = t->tm_hour;
  int minutes = t->tm_min;
  int min_index = minutes;
  
  if(minutes>=33) min_index = 60-minutes; 
  min_index += 2;
  min_index /= 5;

  strcpy(line1, "");
  strcpy(line2, "");
  strcpy(line3, "");

  if (minutes >= 33) hours++;
  if (hours >= 24) hours = 0;
  if (hours > 12) hours -= 12;

  strcat(line1, REL_EN[min_index]);    
  if(min_index==5) strcat(line2, "five ");
  if (minutes == 0) {
    if(hours == 12) {
      strcat(line2, STR_NOON);
      strcat(line3, "!");
      ret = 2;
    }
    else if(hours == 0) {
      strcat(line2, STR_MID);
      strcat(line3, STR_NIGHT);      
    }
    else {
      strcat(line2, HOURS_EN[hours]);
      strcat(line3,STR_OH_CLOCK);
      ret = 2;
     }
  }
  else if (minutes  < 5-2) {
    strcat(line2, STR_ALREADY);
    if(hours == 12) {
      strcat(line3, STR_NOON);
    }
    else /*if(hours == 0)*/ {
      strcat(line3, HOURS_EN[hours]);
    }
  }
  else if (minutes >= 60-2) {
    strcat(line2, STR_ALMOST);
    if(hours == 12) {
      strcat(line3, STR_NOON);
    }
    else {
      strcat(line3, HOURS_EN[hours]);
    }
  }
  else if (minutes < 35-2) {
    strcat(line2, STR_PAST);
    strcat(line3, HOURS_EN[hours]);
  }
  else {
    strcat(line2, STR_TO);
    strcat(line3, HOURS_EN[hours]);
  }

  return ret;
}