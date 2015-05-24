#include <pebble.h>
#include "splash_screen.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_droid_serif_28_bold;
static TextLayer *s_textlayer_1;

static void initialise_ui(void) {
  s_window = window_create();
  
  s_res_droid_serif_28_bold = fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD);
  // s_textlayer_1
  s_textlayer_1 = text_layer_create(GRect(0, 62, 144, 37));
  text_layer_set_text(s_textlayer_1, "rss-news");
  text_layer_set_text_alignment(s_textlayer_1, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_1, s_res_droid_serif_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_1);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_textlayer_1);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_splash_screen(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_splash_screen(void) {
  window_stack_remove(s_window, true);
}
