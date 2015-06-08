#include <pebble.h>
#include "latest_view.h"
#include "main_menu.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

#ifdef PBL_SDK_3
static StatusBarLayer *s_status_bar;
#endif

#define NUM_MENU_SECTIONS 1
static int numMenuItems = 0;

static char *_latest[20];

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
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
        switch (cell_index->row) {
            case 0:
                menu_cell_basic_draw(ctx, cell_layer, NULL, "Back", NULL);
                break;
            default:
              if (numMenuItems > 0) {
                APP_LOG(APP_LOG_LEVEL_DEBUG, _latest[cell_index->row-1]);
                menu_cell_basic_draw(ctx, cell_layer, NULL, _latest[cell_index->row-1], NULL);
              }
              break;
        }
      break;
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
        hide_latest_view();
        break;
    default:
      if (numMenuItems > 0) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, _latest[cell_index->row]);
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

static void destroy_ui(void) {
  window_destroy(s_window);
  menu_layer_destroy(s_menu_layer);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

static void initialise_ui(void) {
  s_window = window_create();
#ifndef PBL_SDK_3
  window_set_fullscreen(s_window, false);
#endif

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
  
#ifdef PBL_SDK_3
  // Set up the status bar last to ensure it is on top of other Layers
  s_status_bar = status_bar_layer_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
#endif
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Initialised latest view ui");
}

static void split_string(const char *latest) {
  char *p;
  char* new_str;
  if (strlen(latest) > 0) {
    new_str = malloc(strlen(&latest[1]));
    strcpy(new_str,&latest[1]);

    APP_LOG(APP_LOG_LEVEL_DEBUG, new_str);
    p = strtok(new_str,"|");
    while(p != NULL) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, p);
      _latest[numMenuItems++] = p;
      p = strtok(NULL, "|");
      if (numMenuItems == 10) {
        break;
      }
    }
    free(new_str);
  }
}

void show_latest_view(char* latest) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, latest);
  split_string(latest);

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