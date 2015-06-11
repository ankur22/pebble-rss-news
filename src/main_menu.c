#include <pebble.h>
#include "main_menu.h"
#include "latest_view.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

#ifdef PBL_SDK_3
static StatusBarLayer *s_status_bar;
#endif

#define NUM_MENU_SECTIONS 1
#define NUM_MENU_ITEMS 3

static char* _top;
static char* _latest;

static int _numLatestItems = 0;
static char *_latestArray[20];
static char *_latestArrayUrl[20];
static char *_latestArraySource[20];
static char *_latestArrayCategory[20];

static int _numTopItems = 0;
static char *_topArray[20];
static char *_topArrayUrl[20];
static char *_topArraySource[20];
static char *_topArrayCategory[20];

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return NUM_MENU_ITEMS;
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
      switch (cell_index->row) {
        case 0:
          menu_cell_basic_draw(ctx, cell_layer, "Latest", "Latest news", NULL);
          break;
        case 1:
          menu_cell_basic_draw(ctx, cell_layer, "Top", "Popular news", NULL);
          break;
        case 2:
          menu_cell_basic_draw(ctx, cell_layer, "Categories", "All categories", NULL);
          break;
      }
      break;
  }
}

static int split_string(char *fullString, char **array, char **arrayUrl, char **arraySource, char **arrayCategory) {
  int num = 0;
  char *p;
  if (strlen(fullString) > 0) {
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
      APP_LOG(APP_LOG_LEVEL_DEBUG, array[num]);
      APP_LOG(APP_LOG_LEVEL_DEBUG, arrayUrl[num]);
      ++num;
    }
  }
  return num;
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  reset_latest_view();
  switch (cell_index->row) {
    case 0:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Selected Latest");
      show_latest_view(_latestArray, _latestArrayUrl, _latestArraySource, _latestArrayCategory, _numLatestItems);
      break;
    case 1:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Selected Top");
      show_latest_view(_topArray, _topArrayUrl, _topArraySource, _topArrayCategory, _numTopItems);
      break;
    case 2:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Selected Categories");
      break;
  }
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      menu_cell_basic_header_draw(ctx, cell_layer, "rss-news");
      break;
  }
}

static void destroy_ui(void) {
  window_destroy(s_window);
  menu_layer_destroy(s_menu_layer);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

static void initialise_ui(void) {
  s_window = window_create();
/*#ifndef PBL_SDK_3
  window_set_fullscreen(s_window, false);
#endif*/
  
Layer *window_layer = window_get_root_layer(s_window);
  
  s_menu_layer = menu_layer_create(GRect(0, 0, 144, 152));
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
  });
  menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
  layer_add_child(window_layer, (Layer *)s_menu_layer);
  
/*#ifdef PBL_SDK_3
  // Set up the status bar last to ensure it is on top of other Layers
  s_status_bar = status_bar_layer_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
#endif*/
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Initialised main menu ui");
}

void show_main_menu_no_params(void) {
    show_main_menu(_top, _latest);
}

void show_main_menu(char* top, char* latest) {
  _top = top;
  _latest = latest;
  _numLatestItems = split_string(_latest, _latestArray, _latestArrayUrl, _latestArraySource, _latestArrayCategory);
  _numTopItems = split_string(_top, _topArray, _topArrayUrl, _topArraySource, _topArrayCategory);

  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Main menu added to window stack");
}

void hide_main_menu(void) {
  window_stack_remove(s_window, true);
}