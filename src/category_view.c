#include <pebble.h>
#include "category_view.h"
#include "latest_view.h"

#define MESSAGE_TYPE 0
#define DOWN_MAIN_MENU 11
#define UP_MAIN_MENU 12
#define GET_HEADLINES 10
#define LINE_HEIGHT 20
#define NUM_CHARS_IN_LINE 26

static Window *s_window;

#ifndef PBL_PLATFORM_APLITE
static StatusBarLayer *s_status_bar;

static GBitmapSequence *s_sequence;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;
#endif

#define MAX_CELL_HEIGHT 95
#define NUM_MENU_SECTIONS 1
static int numMenuItems = 0;
static int selectedMenuCell = 0;

static char _categories[60][50];

static GFont s_res_droid_serif_28_bold;
static TextLayer *s_textlayer_1;
static TextLayer *s_textlayer_2;
static TextLayer *s_textlayer_menu_item;

static int imageNumber = 0;


static int _numLatestItems = 0;
static char *_latestArray[20];
static char *_latestArrayUrl[20];
static char *_latestArraySource[20];
static char *_latestArrayCategory[20];


static void show_animation() {
#ifndef PBL_PLATFORM_APLITE
    layer_set_hidden((Layer *)s_bitmap_layer, false);
#else
    layer_set_hidden((Layer *)s_textlayer_2, false);
#endif
}

static void hide_animation() {
#ifndef PBL_PLATFORM_APLITE
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
    char* fullLine = _categories[cell_index->row];
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
            return row_height(cell_index) + 20;
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

#ifndef PBL_PLATFORM_APLITE
static void menu_draw_row_callback_basalt(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    if (selectedMenuCell == cell_index->row) {
        graphics_context_set_text_color(ctx, GColorWhite);
    } else {
        graphics_context_set_text_color(ctx, GColorBlack);
    }
#ifdef PBL_ROUND
    GRect headline_bounds = GRect(25, 8, 136, row_height(cell_index));
#else
    GRect headline_bounds = GRect(5, 8, 136, row_height(cell_index));
#endif
    graphics_draw_text(ctx, _categories[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), headline_bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}
#else
static void menu_draw_row_callback_aplite(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    if (selectedMenuCell == cell_index->row) {
        graphics_context_set_text_color(ctx, GColorWhite);
    } else {
        graphics_context_set_text_color(ctx, GColorBlack);
    }
    graphics_draw_text(ctx, _categories[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(5, 8, 137, row_height(cell_index)), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}
#endif

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
        if (numMenuItems > 0) {
#ifndef PBL_PLATFORM_APLITE
            menu_draw_row_callback_basalt(ctx, cell_layer, cell_index, data);
#else
            menu_draw_row_callback_aplite(ctx, cell_layer, cell_index, data);
#endif
        }
    break;
  }
}

static int16_t get_menu_separator_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    return 0;
}

static void menu_draw_separator(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
  switch (cell_index->section) {
    case 0:
        if (numMenuItems > 0) {
            //graphics_context_set_stroke_color(ctx, GColorBlack);
            //graphics_draw_line(ctx, GPoint(5, 3), GPoint(139, 3));
        }
    break;
  }
}

static void do_post(char *category, int key) {
    if (bluetooth_connection_service_peek()) {
      APP_LOG(APP_LOG_LEVEL_INFO, "CatView: Phone is connected!");

      DictionaryIterator *iter;
      app_message_outbox_begin(&iter);
      dict_write_cstring(iter, key, category);
      app_message_outbox_send();
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "CatView: Phone is not connected!");
      hide_animation();
      show_no_con_error();
    }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case 0:
      if (numMenuItems > 0) {
        do_post(_categories[cell_index->row], GET_HEADLINES);
        APP_LOG(APP_LOG_LEVEL_DEBUG, _categories[cell_index->row]);
        light_enable_interaction();
        show_animation();
      }
    break;
  }
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
#ifdef PBL_ROUND
      menu_cell_basic_header_draw(ctx, cell_layer, "                    rss-news");
#else
      menu_cell_basic_header_draw(ctx, cell_layer, "rss-news");
#endif
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
  text_layer_destroy(s_textlayer_1);
  text_layer_destroy(s_textlayer_2);
  text_layer_destroy(s_textlayer_menu_item);

#ifndef PBL_PLATFORM_APLITE
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

#ifndef PBL_PLATFORM_APLITE
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

static void showLatestView(char* latest) {
  _numLatestItems = split_string(latest, _latestArray, _latestArrayUrl, _latestArraySource, _latestArrayCategory);
  
//   hide_category_view();
  
  show_latest_view(_latestArray, _latestArrayUrl, _latestArraySource, _latestArrayCategory, _numLatestItems);
  
  light_enable_interaction();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message recieved!");
  hide_animation();
  
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  // Process all pairs present
  while (t != NULL) {
    // Long lived buffer
    static char s_buffer[64];

    // Process this pair's key
    switch (t->key) {
      case GET_HEADLINES:
        snprintf(s_buffer, sizeof(s_buffer), "GET_HEADLINES Received '%s'", t->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, s_buffer);
        showLatestView(t->value->cstring);
        break;
      default:
        snprintf(s_buffer, sizeof(s_buffer), "Unidentified Received '%s'", t->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, s_buffer);
        break;
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

static void handle_window_unload(Window* window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "unload");
  destroy_ui();
}

static void handle_window_appear(Window* window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "appear");
  app_message_register_inbox_received(inbox_received_callback);
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

static void show_menu_item(int item_number) {
  text_layer_set_text(s_textlayer_menu_item, _categories[item_number]);
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    hide_no_con_error();
    
    do_post(_categories[selectedMenuCell], GET_HEADLINES);
    APP_LOG(APP_LOG_LEVEL_DEBUG, _categories[selectedMenuCell]);
    light_enable_interaction();
    show_animation();
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    hide_no_con_error();
    
    ++selectedMenuCell;
    
    if (selectedMenuCell >= numMenuItems) {
	    selectedMenuCell = numMenuItems - 1;
    }
    
    show_menu_item(selectedMenuCell);
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, _categories[selectedMenuCell]);
    light_enable_interaction();
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    hide_no_con_error();
    
    --selectedMenuCell;
    
    if (selectedMenuCell < 0) {
	    selectedMenuCell = 0;
    }
    
    show_menu_item(selectedMenuCell);
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, _categories[selectedMenuCell]);
    light_enable_interaction();
}

static void config_provider(Window *window) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
}

static void initialise_ui(void) {
  s_window = window_create();

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = handle_window_load,
    .unload = handle_window_unload,
    .appear = handle_window_appear,
    .disappear = handle_window_disappear,
  });

  window_set_click_config_provider(s_window, (ClickConfigProvider) config_provider);

  Layer *window_layer = window_get_root_layer(s_window);

  s_res_droid_serif_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  
#ifndef PBL_PLATFORM_APLITE
#ifdef PBL_ROUND
  s_bitmap_layer = bitmap_layer_create(GRect(57, 52, 64, 64));
#else
  s_bitmap_layer = bitmap_layer_create(GRect(37, 52, 64, 64));
#endif
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

  // Create sequence
  s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_LOADING_ANIMATION);
  // Create GBitmap
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);
  // Begin animation
  app_timer_register(1, timer_handler, NULL);
  hide_animation();
#else
  s_textlayer_2 = text_layer_create(GRect(0, 52, 144, 27));
  text_layer_set_text(s_textlayer_2, "Loading, please wait");
  text_layer_set_text_alignment(s_textlayer_2, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_2, s_res_droid_serif_28_bold);
  layer_add_child(window_layer, (Layer *)s_textlayer_2);
  hide_animation();
#endif

#ifdef PBL_ROUND
  s_textlayer_1 = text_layer_create(GRect(0, 52, 180, 27));
#else
  s_textlayer_1 = text_layer_create(GRect(0, 52, 144, 27));
#endif
  text_layer_set_text(s_textlayer_1, "Currently unavailable");
  text_layer_set_text_alignment(s_textlayer_1, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_1, s_res_droid_serif_28_bold);
  layer_add_child(window_layer, (Layer *)s_textlayer_1);
  hide_no_con_error();

  s_textlayer_menu_item = text_layer_create(GRect(0, 52, 144, 27));
  text_layer_set_text(s_textlayer_menu_item, "");
  text_layer_set_text_alignment(s_textlayer_menu_item, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_menu_item, s_res_droid_serif_28_bold);
  layer_add_child(window_layer, (Layer *)s_textlayer_menu_item);
  show_menu_item(0);
  
#ifndef PBL_PLATFORM_APLITE
  // Set up the status bar last to ensure it is on top of other Layers
  s_status_bar = status_bar_layer_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
#endif

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Initialised latest view ui");
}

void show_category_view(char **categories, int num) {
  numMenuItems = num;
  
  for (int i = 0; i < num; ++i) {
  	strcpy(_categories[i], categories[i]);
  }

  app_message_register_inbox_received(inbox_received_callback);

  initialise_ui();
  window_stack_push(s_window, true);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Latest view added to window stack");
}

void hide_category_view(void) {
  window_stack_remove(s_window, true);
}