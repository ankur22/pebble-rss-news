#include <pebble.h>
#include "splash_screen.h"
#include "main_menu.h"
  
#define MESSAGE_TYPE 0
#define GET_LATEST 1
#define GET_TOP 2
#define GET_CATEGORIES 3
#define ALL 4
#define HELLO 5
#define ERROR 6
#define READING_LIST 7

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_droid_serif_28_bold;
static TextLayer *s_textlayer_1;

static GFont s_res_gothic_14;
static TextLayer *s_textlayer_2;

#ifdef PBL_SDK_3
static StatusBarLayer *s_status_bar;

static GBitmapSequence *s_sequence;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;
#endif

static char* _top;
static char* _latest;

#ifdef PBL_SDK_3
static void timer_handler(void *context) {
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
}
#endif

static void show_no_con_error() {
    layer_set_hidden((Layer *)s_textlayer_2, false);
#ifdef PBL_SDK_3
    layer_set_hidden((Layer *)s_bitmap_layer, true);
#endif
}

static void hide_no_con_error() {
    layer_set_hidden((Layer *)s_textlayer_2, true);
}

static void initialise_ui(void) {
  s_window = window_create();
#ifndef PBL_SDK_3
  window_set_fullscreen(s_window, false);
#endif
  Layer *window_layer = window_get_root_layer(s_window);
  
  s_res_droid_serif_28_bold = fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD);
  // s_textlayer_1
  s_textlayer_1 = text_layer_create(GRect(0, 32, 144, 37));
  text_layer_set_text(s_textlayer_1, "rss-news");
  text_layer_set_text_alignment(s_textlayer_1, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_1, s_res_droid_serif_28_bold);
  layer_add_child(window_layer, (Layer *)s_textlayer_1);

  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  s_textlayer_2 = text_layer_create(GRect(0, 95, 144, 27));
  text_layer_set_text(s_textlayer_2, "Currently unavailable");
  text_layer_set_text_alignment(s_textlayer_2, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_2, s_res_gothic_14);
  layer_add_child(window_layer, (Layer *)s_textlayer_2);
  hide_no_con_error();
  
#ifdef PBL_SDK_3
  s_bitmap_layer = bitmap_layer_create(GRect(37, 72, 64, 64));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

  // Create sequence
  s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_LOADING_ANIMATION);
  // Create GBitmap
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);
  // Begin animation
  app_timer_register(1, timer_handler, NULL);

  // Set up the status bar last to ensure it is on top of other Layers
  s_status_bar = status_bar_layer_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
#endif
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_textlayer_1);
  text_layer_destroy(s_textlayer_2);
#ifdef PBL_SDK_3
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
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

static void do_gets(int key) {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  dict_write_int(iter, MESSAGE_TYPE, &key, sizeof(int), true);
  app_message_outbox_send();
}

static void sendDoGetMessageIfBluetoothConnected() {
  if (bluetooth_connection_service_peek()) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Phone is connected!");
    do_gets(ALL);
  } else {
    APP_LOG(APP_LOG_LEVEL_INFO, "Phone is not connected!");
  }
}

static void displayMainMenu(char *_top, char *_latest) {
  hide_splash_screen();
  show_main_menu(_top, _latest);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Main menu has been set to display");
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Message recieved!");
  
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  // Process all pairs present
  while (t != NULL) {
    // Long lived buffer
    static char s_buffer[64];

    // Process this pair's key
    switch (t->key) {
      case ALL:
        snprintf(s_buffer, sizeof(s_buffer), "ALL Received '%s'", t->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, s_buffer);
        displayMainMenu(_top, _latest);
        break;
      case GET_LATEST:
        snprintf(s_buffer, sizeof(s_buffer), "GET_LATEST Received '%s'", t->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, s_buffer);
        _latest = t->value->cstring;
        break;
      case GET_TOP:
        snprintf(s_buffer, sizeof(s_buffer), "GET_TOP Received '%s'", t->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, s_buffer);
        _top = t->value->cstring;
        break;
      case HELLO:
        snprintf(s_buffer, sizeof(s_buffer), "HELLO Received '%s'", t->value->cstring);
        APP_LOG(APP_LOG_LEVEL_DEBUG, s_buffer);
        sendDoGetMessageIfBluetoothConnected();
        break;
      case ERROR:
        snprintf(s_buffer, sizeof(s_buffer), "ERROR Received '%s'", t->value->cstring);
        APP_LOG(APP_LOG_LEVEL_ERROR, s_buffer);
        show_no_con_error();
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

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  char str[100];
  snprintf(str, sizeof(str), "Message dropped! Reason: %d", reason);
  APP_LOG(APP_LOG_LEVEL_ERROR, str);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  char str[100];
  snprintf(str, sizeof(str), "Outbox send failed! Reason: %d", reason);
  APP_LOG(APP_LOG_LEVEL_ERROR, str);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void registerCallbacks(void) {
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
}

static void openAppMessage(void) {
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

void show_splash_screen(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded splash screen");
  
  registerCallbacks();
  openAppMessage();
}

void hide_splash_screen(void) {
  window_stack_remove(s_window, true);
}