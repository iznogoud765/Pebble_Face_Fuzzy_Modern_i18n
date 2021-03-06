#include <pebble.h>
#include "fuzzy_time.h"
#include "time_fr.h"
#include "time_en.h"
#include "time_de.h"
#include "time_es.h"


#define LINE_BUFFER_SIZE 50


int fuzzy_time(struct tm* t, char* line1, char* line2, char* line3) {
  int ret = 1;
  
  int hours = t->tm_hour;
  int minutes = t->tm_min;

  if (minutes >= 33) hours++;
  if (hours >= 24) hours = 0;
  if (hours > 12) hours -= 12;

#if DEBUG == 2
  const char* str_lang = setlocale(LC_ALL, "de_DE");
#elif DEBUG == 3
  const char* str_lang = setlocale(LC_ALL, "es_ES");
#elif DEBUG == 4
  const char* str_lang = setlocale(LC_ALL, "fr_FR");
#else
  const char* str_lang =  i18n_get_system_locale();
#endif
  
  if (strcmp("fr_FR", str_lang) == 0) {
    ret=french_time(t, line1, line2, line3); //
  }
  else if (strcmp("de_DE", str_lang) == 0) {
    ret=fuzzy_time_de(t, line1, line2, line3); //
  }
  else if (strcmp("es_ES", str_lang) == 0) {
    ret=fuzzy_time_es(t, line1, line2, line3); //
  } else {
    // Fall back to English
    ret=fuzzy_time_en(t, line1, line2, line3); //
}

  return ret;
}

void info_lines(struct tm* t, char* line1, char* line2) {
  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(line1, LINE_BUFFER_SIZE, "%H:%M", t);
  } else {
    // Use 12 hour format
    strftime(line1, LINE_BUFFER_SIZE, "%I:%M %p", t);
  }
  
  const char* str_lang =  i18n_get_system_locale();
  const char* str_datefmt = "%A %e %B";
  // Write the current date into the buffer
#ifdef PBL_PLATFORM_CHALK
  if (strcmp("en_US", str_lang) == 0) 
    str_datefmt = "%m/%d";
  else
    str_datefmt = "%d/%m";
#else
  if (strcmp("en_US", str_lang) == 0) 
    str_datefmt = "%a, %B %e";
  else if (strcmp("es_ES", str_lang) == 0) 
    str_datefmt = "%a, %e de %B";
  else if (strcmp("de_DE", str_lang) == 0) 
    str_datefmt = "%a, den %e. %B";
  else
    str_datefmt = "%A %e %B";
#endif    
  strftime(line2, LINE_BUFFER_SIZE, str_datefmt, t);

/*  
  char buf[LINE_BUFFER_SIZE];
  strftime(buf, LINE_BUFFER_SIZE, "%Ec", t);
  APP_LOG(APP_LOG_LEVEL_DEBUG , "date: %s", buf);
  strftime(buf, LINE_BUFFER_SIZE, "%r", t);
  APP_LOG(APP_LOG_LEVEL_DEBUG , "time: %s", buf);
*/
}