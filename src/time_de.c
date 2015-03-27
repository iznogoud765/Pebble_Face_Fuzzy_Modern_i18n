#include <pebble.h>
#include "time_de.h"
#include "string.h"
  
static const char* const STUNDEN[] = {
  "zwölf",
  "eins",
  "zwei",
  "drei",
  "vier",
  "fünf",
  "sechs",
  "sieben",
  "acht",
  "neun",
  "zehn",
  "elf"
};

int fuzzy_time_de(struct tm* t, char* line1, char* line2, char* line3)
{
  int ret = 3;
  
  int hours = t->tm_hour;
  int minutes = t->tm_min;

  strcpy(line1, "");
  strcpy(line2, "");
  strcpy(line3, "");

  if (minutes == 0) {
    if (hours == 0) {
      strcat(line1, "mitter-nacht");
    } else { 
      if (hours % 12 == 1) {
        strcat(line1, "ein");
      } else {
        strcat(line1, STUNDEN[hours % 12]);
      }
      strcat(line2, "uhr");
      if (hours < 5 || hours > 21) {
        strcat(line3, "nachts");
      } else if ( hours < 12) {
        strcat(line3, "morgens");
      } else if ( hours < 13) {
        strcat(line3, "mittags");
      } else if ( hours < 18) {
        strcat(line3, "nach-mittags");
      } else {
        strcat(line3, "abends");
      }
    }
    ret = 1;
  }  
  else if (minutes <= 3) {
      strcat(line1, "kurz");
      strcat(line2, "nach");
  } else if (minutes <= 8) {
      strcat(line1, "fünf");
      strcat(line2, "nach");
  } else if (minutes <= 12) {
      strcat(line1, "zehn");
      strcat(line2, "nach");
  } else if (minutes <= 17) {
      strcat(line1, "viertel");
      strcat(line2, "nach");
  } else if (minutes <= 22) {
      strcat(line1, "zwanzig");
      strcat(line2, "nach");
  } else if (minutes <= 28) {
      strcat(line1, "fünf");
      strcat(line2, "vor halb");
  } else if (minutes <= 29) {
      strcat(line1, "kurz");
      strcat(line2, "vor halb");
  } else if (minutes <= 30) {
      strcat(line1, "halb");
  } else if (minutes <= 32) {
      strcat(line1, "kurz");
      strcat(line2, "nach halb");
  } else if (minutes <= 37) {
      strcat(line1, "fünf");
      strcat(line2, "nach halb");
  } else if (minutes <= 43) {
      strcat(line1, "zwanzig");
      strcat(line2, "vor");
  } else if (minutes <= 48) {
      strcat(line1, "viertel");
      strcat(line2, "vor");
  } else if (minutes <= 53) {
      strcat(line1, "zehn");
      strcat(line2, "vor");
  } else if (minutes <= 57) {
      strcat(line1, "fünf");
      strcat(line2, "vor");
  } else if (minutes <= 59) {
      strcat(line1, "kurz");
      strcat(line2, "vor");
  }
  if (minutes > 23) hours++;
  strcat(line3, STUNDEN[hours % 12]);
  
  return ret;
}