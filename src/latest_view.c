#include <pebble.h>
#include "latest_view.h"
#include "main_menu.h"

#define MESSAGE_TYPE 0
#define READING_LIST 7
#define LINE_HEIGHT 15
#define NUM_CHARS_IN_LINE 28

static Window *s_window;
static MenuLayer *s_menu_layer;

#ifdef PBL_SDK_3
static StatusBarLayer *s_status_bar;
#endif

#define MAX_CELL_HEIGHT 95
#define NUM_MENU_SECTIONS 1
static int numMenuItems = 0;
static int selectedMenuCell = 0;

static char **_latest;
static char **_latestUrl;
static char **_latestSource;
static char **_latestCategory;

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static int16_t num_lines(MenuIndex *cell_index) {
    char* fullLine = _latest[cell_index->row];
    int length = strlen(fullLine);
    int num_lines = 0;
    int count = 0;
    int word = 0;
    for (int i = 0; i < length; ++i) {
        char* character = &fullLine[i];
        count += 1;
        word += 1;
        if (*character == ' ') {
            word = 0;
        }
        if (count > NUM_CHARS_IN_LINE) {
            i -= word;
            word = 0;
            count = 0;
            num_lines += 1;
        }
    }
    return num_lines;
}

static int16_t row_height(MenuIndex *cell_index) {
    return LINE_HEIGHT * num_lines(cell_index);
}

static int16_t wp_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
        if (numMenuItems > 0) {
            return row_height(cell_index) + 50;
        }
    break;
  }
  return MAX_CELL_HEIGHT;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return numMenuItems;
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
        if (numMenuItems > 0) {
            //menu_cell_basic_draw(ctx, cell_layer, NULL, _latest[cell_index->row], NULL);
            if (selectedMenuCell == cell_index->row) {
                graphics_context_set_text_color(ctx, GColorWhite);
            } else {
                graphics_context_set_text_color(ctx, GColorBlack);
            }
            graphics_draw_text(ctx, _latest[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(5, 0, 139, row_height(cell_index)), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
#ifdef PBL_SDK_3
            graphics_context_set_text_color(ctx, GColorLightGray);
#endif
            graphics_draw_text(ctx, _latestSource[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(10, row_height(cell_index) + 15, 139, 10), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
#ifdef PBL_SDK_3
            graphics_context_set_text_color(ctx, GColorVividCerulean);
#endif
            graphics_draw_text(ctx, _latestCategory[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(10, row_height(cell_index) + 30, 139, 10), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
        }
    break;
  }
}

static int16_t get_menu_separator_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    return 5;
}

static void menu_draw_separator(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  switch (cell_index->section) {
    case 0:
        if (numMenuItems > 0) {
            graphics_context_set_stroke_color(ctx, GColorBlack);
            graphics_draw_line(ctx, GPoint(5, 3), GPoint(139, 3));
        }
    break;
  }
}

static void do_post(char *url) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_cstring(iter, MESSAGE_TYPE, url);
  app_message_outbox_send();
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
      if (numMenuItems > 0) {
        do_post(_latestUrl[cell_index->row]);
        APP_LOG(APP_LOG_LEVEL_DEBUG, _latestUrl[cell_index->row]);
        light_enable_interaction();
      }
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

static void menu_selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context) {
  switch (new_index.section) {
    case 0:
      selectedMenuCell = new_index.row;
      light_enable_interaction();
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
    .get_cell_height = wp_cell_height,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
    .draw_separator = menu_draw_separator,
    .get_separator_height = get_menu_separator_height,
    .selection_changed = menu_selection_changed,
  });
  menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
  layer_add_child(window_layer, (Layer *)s_menu_layer);
  
/*#ifdef PBL_SDK_3
  // Set up the status bar last to ensure it is on top of other Layers
  s_status_bar = status_bar_layer_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
#endif*/
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Initialised latest view ui");
}

void show_latest_view(char **latest, char **latestUrl, char **latestSource, char **latestCategory, int num) {
  numMenuItems = num;
  _latest = latest;
  _latestUrl = latestUrl;
  _latestSource = latestSource;
  _latestCategory = latestCategory;

  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Latest view added to window stack");
}

void hide_latest_view(void) {
  window_stack_remove(s_window, true);
}

void reset_latest_view(void) {
  //numMenuItems = 0;
  selectedMenuCell = 0;
  //menu_layer_reload_data(s_menu_layer);
}