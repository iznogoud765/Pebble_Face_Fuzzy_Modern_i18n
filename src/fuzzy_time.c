#include <pebble.h>
#include "fuzzy_time.h"
#include "french_time.h"


#define LINE_BUFFER_SIZE 50


int fuzzy_time(struct tm* t, char* line1, char* line2, char* line3) {
  int ret = 1;
  
  int hours = t->tm_hour;
  int minutes = t->tm_min;

  if (minutes >= 33) hours++;
  if (hours >= 24) hours = 0;
  if (hours > 12) hours -= 12;

  const char* str_lang =  i18n_get_system_locale();
  
  if (strcmp("fr_FR", str_lang) == 0) {
   ret=french_time(t, line1, line2, line3); //
  /*
  }
  else if (strcmp("de_DE", str_lang) == 0) {
  ; //
  }
  else if (strcmp("es_ES", str_lang) == 0) {
  ; //
  */
  } else {
    // Fall back to English
   ret=french_time(t, line1, line2, line3); //
}

  return ret;
}

void info_lines(struct tm* t, char* line1, char* line2) {
  strftime(line1, LINE_BUFFER_SIZE, "%H:%M", t);

  strftime(line2, LINE_BUFFER_SIZE, "%A %e %B", t);
}