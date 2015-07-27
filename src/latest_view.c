#include <pebble.h>
#include "latest_view.h"
#include "main_menu.h"

#define MESSAGE_TYPE 0
#define SAVED_READING_LIST 8
#define READING_LIST 7
#define LINE_HEIGHT 15
#define NUM_CHARS_IN_LINE 28

static Window *s_window;
static MenuLayer *s_menu_layer;

#ifdef PBL_PLATFORM_BASALT
static StatusBarLayer *s_status_bar;

static GBitmapSequence *s_sequence;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;
#endif

#define MAX_CELL_HEIGHT 95
#define NUM_MENU_SECTIONS 1
static int numMenuItems = 0;
static int selectedMenuCell = 0;
static int selectedMenuCellSubAdd = 1;

static char **_latest;
static char **_latestUrl;
static char **_latestSource;
static char **_latestCategory;
static char **_username;

static GFont s_res_droid_serif_28_bold;
static TextLayer *s_textlayer_1;
static TextLayer *s_textlayer_2;

static void show_animation() {
#ifdef PBL_PLATFORM_BASALT
    layer_set_hidden((Layer *)s_bitmap_layer, false);
#else
    layer_set_hidden((Layer *)s_textlayer_2, false);
#endif
}

static void hide_animation() {
#ifdef PBL_PLATFORM_BASALT
    layer_set_hidden((Layer *)s_bitmap_layer, true);
#else
    layer_set_hidden((Layer *)s_textlayer_2, true);
#endif
}

static void show_no_con_error() {
    layer_set_hidden((Layer *)s_textlayer_1, false);
}

static void hide_no_con_error() {
    layer_set_hidden((Layer *)s_textlayer_1, true);
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static int16_t num_lines(MenuIndex *cell_index) {
    char* fullLine = _latest[cell_index->row];
    int length = strlen(fullLine);

    /*static char s_buffer[128];
    snprintf(s_buffer, sizeof(s_buffer), "num_lines called for '%d'|%s|%d", cell_index->row, fullLine, length);
    APP_LOG(APP_LOG_LEVEL_DEBUG, s_buffer);*/

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
    if (num_lines == 0 && length > 0) {
        num_lines = 1;
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

#ifdef PBL_PLATFORM_BASALT
static void menu_draw_row_callback_basalt(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    //menu_cell_basic_draw(ctx, cell_layer, NULL, _latest[cell_index->row], NULL);
    if (selectedMenuCell == cell_index->row) {
        graphics_context_set_text_color(ctx, GColorWhite);
    } else {
        graphics_context_set_text_color(ctx, GColorBlack);
    }
    graphics_draw_text(ctx, _latest[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(5, 0, 139, row_height(cell_index)), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

    graphics_context_set_text_color(ctx, GColorLightGray);
    graphics_draw_text(ctx, _latestSource[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(10, row_height(cell_index) + 15, 139, 10), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

    graphics_context_set_text_color(ctx, GColorVividCerulean);
    graphics_draw_text(ctx, _latestCategory[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(10, row_height(cell_index) + 30, 139, 10), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}
#else
static void menu_draw_row_callback_aplite(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    //menu_cell_basic_draw(ctx, cell_layer, NULL, _latest[cell_index->row], NULL);
    if (selectedMenuCell == cell_index->row+selectedMenuCellSubAdd) {
        graphics_context_set_text_color(ctx, GColorWhite);
    } else {
        graphics_context_set_text_color(ctx, GColorBlack);
    }
    graphics_draw_text(ctx, _latest[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(5, 0, 139, row_height(cell_index)), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, _latestSource[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(10, row_height(cell_index) + 15, 139, 10), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, _latestCategory[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(10, row_height(cell_index) + 30, 139, 10), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}
#endif

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
        if (numMenuItems > 0) {
#ifdef PBL_PLATFORM_BASALT
            menu_draw_row_callback_basalt(ctx, cell_layer, cell_index, data);
#else
            menu_draw_row_callback_aplite(ctx, cell_layer, cell_index, data);
#endif
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
    if (bluetooth_connection_service_peek()) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Phone is connected!");

      DictionaryIterator *iter;
      app_message_outbox_begin(&iter);
      dict_write_cstring(iter, MESSAGE_TYPE, url);
      app_message_outbox_send();
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "Phone is not connected!");
      hide_animation();
      show_no_con_error();
    }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
      if (numMenuItems > 0) {
        do_post(_latestUrl[cell_index->row]);
        APP_LOG(APP_LOG_LEVEL_DEBUG, _latestUrl[cell_index->row]);
        APP_LOG(APP_LOG_LEVEL_DEBUG, _latest[cell_index->row]);
        light_enable_interaction();
        show_animation();
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
#ifdef PBL_PLATFORM_BASALT
      selectedMenuCell = new_index.row;
#else
      selectedMenuCell = old_index.row;
      selectedMenuCellSubAdd = new_index.row - old_index.row;
#endif
      light_enable_interaction();
      break;
  }
}

static void destroy_ui(void) {
  window_destroy(s_window);
  menu_layer_destroy(s_menu_layer);
  text_layer_destroy(s_textlayer_1);
  text_layer_destroy(s_textlayer_2);
//  free(_latest);
//  free(_latestUrl);
//  free(_latestSource);
//  free(_latestCategory);

#ifdef PBL_PLATFORM_BASALT
  if(s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }
  if(s_sequence) {
    gbitmap_sequence_destroy(s_sequence);
    s_sequence = NULL;
  }
  bitmap_layer_destroy(s_bitmap_layer);
#endif
}

#ifdef PBL_PLATFORM_BASALT
static void timer_handler(void *context) {
//  if (layer_get_hidden((Layer *)s_bitmap_layer) == false) {
      uint32_t next_delay;

      // Advance to the next APNG frame
      if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
        bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
        layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));

        // Timer for that delay
        app_timer_register(next_delay, timer_handler, NULL);
      } else {
        // Start again
        gbitmap_sequence_restart(s_sequence);
      }
//  }
}
#endif

static void handle_window_unload(Window* window) {
  destroy_ui();
}

static void handle_window_appear(Window* window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "appear");
  hide_animation();
}

static void handle_window_disappear(Window* window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "disappear");
  hide_animation();
}

static void handle_window_load(Window* window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "load");
  hide_animation();
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    hide_no_con_error();
}

void config_provider(Window *window) {
  window_single_click_subscribe(BUTTON_ID_SELECT, down_single_click_handler);
}

static void initialise_ui(void) {
  s_window = window_create();
#ifndef PBL_PLATFORM_BASALT
  window_set_fullscreen(s_window, false);
#endif

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = handle_window_load,
    .unload = handle_window_unload,
    .appear = handle_window_appear,
    .disappear = handle_window_disappear,
  });

  window_set_click_config_provider(s_window, (ClickConfigProvider) config_provider);

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

  s_res_droid_serif_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  
#ifdef PBL_PLATFORM_BASALT
  // Set up the status bar last to ensure it is on top of other Layers
  s_status_bar = status_bar_layer_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
#endif
#ifdef PBL_PLATFORM_BASALT
  s_bitmap_layer = bitmap_layer_create(GRect(37, 52, 64, 64));
  layer_insert_above_sibling(bitmap_layer_get_layer(s_bitmap_layer), (Layer *)s_menu_layer);

  // Create sequence
  s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_LOADING_ANIMATION);
  // Create GBitmap
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);
  // Begin animation
  app_timer_register(1, timer_handler, NULL);
  hide_animation();
#else
  s_textlayer_2 = text_layer_create(GRect(0, 52, 144, 27));
  text_layer_set_text(s_textlayer_2, "Saving, please wait");
  text_layer_set_text_alignment(s_textlayer_2, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_2, s_res_droid_serif_28_bold);
  layer_insert_above_sibling((Layer *)s_textlayer_2, (Layer *)s_menu_layer);
  hide_animation();
#endif

  s_textlayer_1 = text_layer_create(GRect(0, 52, 144, 27));
  text_layer_set_text(s_textlayer_1, "Currently unavailable");
  text_layer_set_text_alignment(s_textlayer_1, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_1, s_res_droid_serif_28_bold);
  layer_insert_above_sibling((Layer *)s_textlayer_1, (Layer *)s_menu_layer);
  hide_no_con_error();
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Initialised latest view ui");
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  hide_animation();
}

void show_latest_view(char **latest, char **latestUrl, char **latestSource, char **latestCategory, int num) {
  numMenuItems = num;
  _latest = latest;
  _latestUrl = latestUrl;
  _latestSource = latestSource;
  _latestCategory = latestCategory;

  app_message_register_inbox_received(inbox_received_callback);

  initialise_ui();
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