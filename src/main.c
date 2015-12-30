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

void rss_news_init() {
    show_splash_screen();
}

static void rss_news_deinit() {
    hide_splash_screen();
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &rss_news_init,
    .deinit_handler = &rss_news_deinit,
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 64,
        .outbound = 16,
      }
    }
  };
  app_event_loop(params, &handlers);
}