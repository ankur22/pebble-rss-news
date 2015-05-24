#include <pebble.h>
#include "splash_screen.h"
  
static const int Y_CENTER = 62; 

Window *window;
TextLayer *text_layer;

void handle_splash_screen_init(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Loading splash screen");
  
	// Create a window and text layer
	window = window_create();
  text_layer = text_layer_create(GRect(0, Y_CENTER, 144, 154));
	
	// Set the text, font, and text alignment
	text_layer_set_text(text_layer, "rss-news");
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	
	// Add the text and bitmap layers to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));

	// Push the window
	window_stack_push(window, true);
	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Splash screen loaded");
}

void handle_splash_screen_deinit(void) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Deleting splash screen");
  
	// Destroy the text layer
	text_layer_destroy(text_layer);
	
	// Destroy the window
	window_destroy(window);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Splash screen deleted");
}
