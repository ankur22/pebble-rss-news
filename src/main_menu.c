#include <pebble.h>
#include "main_menu.h"
//#include "latest_view.h"
#include "category_view.h"

/*
static int _numLatestItems = 0;
static char *_latestArray[20];
static char *_latestArrayUrl[20];
static char *_latestArraySource[20];
static char *_latestArrayCategory[20];

static int split_string(char *fullString, char **array, char **arrayUrl, char **arraySource, char **arrayCategory) {
  int num = 0;
  char *p;
  if (fullString != NULL && strlen(fullString) > 0) {
    p = strtok(fullString,"|");
    while(p != NULL) {
      array[num] = p;
      p = strtok(NULL, "|");
      arrayUrl[num] = p;
      p = strtok(NULL, "|");
      arraySource[num] = p;
      p = strtok(NULL, "|");
      arrayCategory[num] = p;
      p = strtok(NULL, "|");
      ++num;
    }
  }
  return num;
}

void show_main_menu(char* latest) {
  _numLatestItems = split_string(latest, _latestArray, _latestArrayUrl, _latestArraySource, _latestArrayCategory);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded main menu");

  show_latest_view(_latestArray, _latestArrayUrl, _latestArraySource, _latestArrayCategory, _numLatestItems);
  light_enable_interaction();
}
*/

static int _numCategories = 0;
static char *_categoryArray[20];

static int split_string(char *fullString, char **array) {
  int num = 0;
  char *p;
  if (fullString != NULL && strlen(fullString) > 0) {
    p = strtok(fullString,"|");
    while(p != NULL) {
      array[num] = p;
      p = strtok(NULL, "|");
      ++num;
    }
  }
  return num;
}

void show_main_menu(char* categories) {
  _numCategories = split_string(categories, _categoryArray);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded main menu");

  show_latest_view(_categoryArray, _numCategories);
  light_enable_interaction();
}