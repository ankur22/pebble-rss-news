#include <pebble.h>

Window *window;

TextLayer *text_layer;

BitmapLayer *s_bitmap_layer;
GBitmap *rssNewsLogo;

void handle_init(void) {
  // Load image
  rssNewsLogo = gbitmap_create_with_resource(RESOURCE_ID_RSS_NEWS_MENU_LOGO);
  
  // 
  s_bitmap_layer = bitmap_layer_create(GRect(5, 5, 28, 28));
  bitmap_layer_set_bitmap(s_bitmap_layer, rssNewsLogo);
  
	// Create a window and text layer
	window = window_create();
  text_layer = text_layer_create(GRect(0, 0, 144, 154));
	
	// Set the text, font, and text alignment
	text_layer_set_text(text_layer, "Hi, I'm a Pebble!");
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	
	// Add the text and bitmap layers to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bitmap_layer));

	// Push the window
	window_stack_push(window, true);
	
	// App Logging!
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Just pushed a window!");
}

void handle_deinit(void) {
  // Destroy the bitmap data
  gbitmap_destroy(rssNewsLogo);
  bitmap_layer_destroy(s_bitmap_layer);
  
	// Destroy the text layer
	text_layer_destroy(text_layer);
	
	// Destroy the window
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
