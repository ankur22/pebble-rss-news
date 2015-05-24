#include <pebble.h>
#include "splash_screen.h"

// TODO: Get data for main menu
// TODO: Display main menu and data
// TODO: Button clicks to interact with main menu
// TODO: On button click open either latest, top or categories
// TODO: Get latest data
// TODO: Display latest data
// TODO: Get top data
// TODO: Display top data
// TODO: Get categories data
// TODO: Display categories data

int main(void) {
	handle_splash_screen_init();
	app_event_loop();
	handle_splash_screen_deinit();
}
