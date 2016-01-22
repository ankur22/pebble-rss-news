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
  
  s_textlayer_menu_item = text_layer_create(GRect(0, 52, 144, 27));
  text_layer_set_text(s_textlayer_menu_item, "");
  text_layer_set_text_alignment(s_textlayer_menu_item, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_menu_item, s_res_droid_serif_28_bold);
  layer_add_child(window_layer, (Layer *)s_textlayer_menu_item);
  show_menu_item(0);
  
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