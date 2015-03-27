#include <pebble.h>
#include "time_es.h"

static const char* STR_JUST = "en";
static const char* STR_PAST = "y";
static const char* STR_TO = "menos";
static const char* STR_TWENTY = " veinti-";

const char* const HOURS_ES[] = {
  // AM hours
  "doce",
  "una",
  "dos",
  "tres",
  "cuatro",
  "cinco",
  "seis",
  "siete",
  "ocho",
  "nueve",
  "diez",
  "once",
  // PM hours
  "doce",
  "una",
  "dos",
  "tres",
  "cuatro",
  "cinco",
  "seis",
  "siete",
  "ocho",
  "nueve",
  "diez",
  "once"
};


const char* const RELS_ES[] = {
  "punto",
  "cinco",
  "diez",
  "cuarto",
  "veinte",
  "veinti-5",
  "media",
  "..."
  };



int fuzzy_time_es(struct tm* t, char* line1, char* line2, char* line3) {
  int ret = 1;
  
  int hours = t->tm_hour;
  int minutes = t->tm_min;

  strcpy(line1, "");
  strcpy(line2, "");
  strcpy(line3, "");


  if (minutes >= 33) hours++;
  if (hours >= 24) hours = 0;

  strcat(line1, HOURS_ES[hours]); // hour

  if (minutes == 0) {
    strcat(line2, STR_JUST); // just !
    strcat(line3, RELS_ES[0]); // o'clock !
  }
  else if (minutes < 8) {
    strcat(line2, STR_PAST);
    strcat(line3, RELS_ES[1]); // 5
  }
  else if (minutes < 13) {
    strcat(line2, STR_PAST);
    strcat(line3, RELS_ES[2]); // 10
  }
  else if (minutes < 18) {
    strcat(line2, STR_PAST);
    strcat(line3, RELS_ES[3]); // 15
  }
  else if (minutes < 23) {
    strcat(line2, STR_PAST);
    strcat(line3, RELS_ES[4]); // 20
  }
  else if (minutes < 28) {
    strcat(line2, STR_PAST);
    strcat(line2, STR_TWENTY);
    strcat(line3, RELS_ES[1]); // 25
  }
  else if (minutes < 33) {
    strcat(line2, STR_PAST);
    strcat(line3, RELS_ES[6]); // 30
  }
  else if (minutes < 38) {
    strcat(line2, STR_TO);  // -
    strcat(line3, RELS_ES[5]); // 25
  }
  else if (minutes < 43) {
    strcat(line2, STR_TO);  // -
    strcat(line3, RELS_ES[4]); // 20
  }
  else if (minutes < 48) {
    strcat(line2, STR_TO);  // -
    strcat(line3, RELS_ES[3]); // 15
  }
  else if (minutes < 53) {
    strcat(line2, STR_TO);  // -
    strcat(line3, RELS_ES[2]); // 10
  }
  else if (minutes < 58) {
    strcat(line2, STR_TO);  // -
    strcat(line3, RELS_ES[2]); // 5
  }
  else if (minutes >= 58) {
    strcat(line1, RELS_ES[7]); // almost
    strcat(line2, HOURS_ES[hours]); // hour
    ret = 2;
  }
  return ret;
}
