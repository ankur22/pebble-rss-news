#include <pebble.h>
#include "latest_view.h"

#define READING_LIST 7
#define LINE_HEIGHT 15
#define NUM_CHARS_IN_LINE 26

static Window *s_window;
static MenuLayer *s_menu_layer;

#ifndef PBL_PLATFORM_APLITE
StatusBarLayer *s_status_bar;

GBitmapSequence *s_sequence;
GBitmap *s_bitmap;
BitmapLayer *s_bitmap_layer;
#endif

#define MAX_CELL_HEIGHT 95
#define NUM_MENU_SECTIONS 1
int numMenuItems = 0;
int selectedMenuCell = 0;

char **_latest;
char **_latestUrl;
char **_latestSource;
char **_latestCategory;

GFont s_res_droid_serif_28_bold;
TextLayer *s_textlayer_1;
TextLayer *s_textlayer_2;

GBitmap *bbc_image1;
GBitmap *bbc_image2;
GBitmap *bbc_image3;
int imageNumber = 0;

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
            return row_height(cell_index) + 81;
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

static GBitmap* getImageIfForSource(char* source) {
    if (strcmp(source, "BBC News - Technol...") == 0 || strcmp(source, "BBC News - Home") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_BBC_LOGO);
    } else if (strcmp(source, "TechCrunch") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_TC_LOGO);
    } else if (strcmp(source, "RT - Daily news") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_RT_LOGO);
    } else if (strcmp(source, "New on MIT Technol...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_MIT_LOGO);
    } else if (strcmp(source, "Wired.co.uk") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_WIRED_LOGO);
    } else if (strcmp(source, "TIME") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_TIME_LOGO);
    } else if (strcmp(source, "The New Yorker: Bu...") == 0 || strcmp(source, "The New Yorker: Hu...") == 0 || strcmp(source, "The New Yorker: Ne...") == 0 || strcmp(source, "The New Yorker: Sc...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_TNY_LOGO);
    } else if (strcmp(source, "RollingStone.com: ...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_RS_LOGO);
    } else if (strcmp(source, "Al Jazeera English") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_AJ_LOGO);
    } else if (strcmp(source, "Forbes - Tech") == 0 || strcmp(source, "Forbes.com: Most p...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_FORBES_LOGO);
    } else if (strcmp(source, "Engadget RSS Feed") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_ENGADGET_LOGO);
    } else if (strcmp(source, "CNN.com - Top Sto...") == 0 || strcmp(source, "CNN.com - Top Stor...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_CNN_LOGO);
    } else if (strcmp(source, "XINHUANEWS") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_XN_LOGO);
    } else if (strcmp(source, "Sports: Sports New...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_WP_LOGO);
    } else if (strcmp(source, "Bits") == 0 || strcmp(source, "NYT > Technology") == 0 || strcmp(source, "NYT > Personal Tec...") == 0 || strcmp(source, "NYT > Internationa...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_NYT_LOGO);
    } else if (strcmp(source, "CNET News") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_CNET_LOGO);
    } else if (strcmp(source, "InfoQ") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_INFOQ_LOGO);
    } else if (strcmp(source, "xkcd.com") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_XKCD_LOGO);
    } else if (strcmp(source, "Dilbert Daily Stri...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_DILBERT_LOGO);
    } else if (strcmp(source, "WSJ.com: WSJD") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_WSJ_LOGO);
    } else if (strcmp(source, "The Oatmeal - Comi...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_OATMEAL_LOGO);
    } else if (strcmp(source, "Empire News") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_EMPIRE_LOGO);
    } else if (strcmp(source, "JPost.com - New-Tech") == 0 || strcmp(source, "JPost.com - Home") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_JPOST_LOGO);
    } else if (strcmp(source, "- Sport RSS Feed") == 0 || strcmp(source, "- Arts and Enterta...") == 0 || strcmp(source, "- News RSS Feed") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_INDEPENDENT_LOGO);
    } else if (strcmp(source, "Gizmodo") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_GIZMODO_LOGO);
    } else if (strcmp(source, "Business and finan...") == 0 || strcmp(source, "Science and techno...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_ECONOMIST_LOGO);
    } else if (strcmp(source, "Technology - The G...") == 0 || strcmp(source, "Network Front - Th...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_GUARDIAN_LOGO);
    } else if (strcmp(source, "World News - Break...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_SKY_LOGO);
    } else if (strcmp(source, "ESPN.com") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_ESPN_LOGO);
    } else if (strcmp(source, "The Verge -  All P...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_VERGE_LOGO);
    } else if (strcmp(source, "Penny Arcade") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_PA_LOGO);
    } else if (strcmp(source, "Repubblica.it > Ho...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_RIT_LOGO);
    } else if (strcmp(source, "SPIEGEL ONLINE - S...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_SPIEGEL_LOGO);
    } else if (strcmp(source, "CNN CHILE") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_CNNC_LOGO);
    } else if (strcmp(source, "Actu : Toute l'act...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_LMFR_LOGO);
    } else if (strcmp(source, "Lonely Planet blog") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_LP_LOGO);
    } else if (strcmp(source, "World news") == 0 || strcmp(source, "Finance - Business...") == 0 || strcmp(source, "Sport") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_TELEGRAPH_LOGO);
    } else if (strcmp(source, "Kotaku") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_KOTAKU_LOGO);
    } else if (strcmp(source, "Times of India") == 0 || strcmp(source, "Technology News, L...") == 0 || strcmp(source, "The Times of India...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_TOI_LOGO);
    } else if (strcmp(source, "Video Game News at...") == 0 || strcmp(source, "New Video Game Rel...") == 0 || strcmp(source, "Game Reviews on Gi...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_GB_LOGO);
    } else if (strcmp(source, "TechRadar: Technol...") == 0 || strcmp(source, "Techradar - All th...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_TR_LOGO);
    } else if (strcmp(source, "Reuters: Top News") == 0 || strcmp(source, "Reuters: Technolog...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_REUTERS_LOGO);
    } else if (strcmp(source, "GameTrailers.com V...") == 0 || strcmp(source, "http://www.gametra...") == 0 || strcmp(source, "GameTrailers.com P...") == 0 || strcmp(source, "GameTrailers.com R...") == 0 || strcmp(source, "GameTrailers.com X...") == 0) {
        return gbitmap_create_with_resource(RESOURCE_ID_GT_LOGO);
    } else {
        return gbitmap_create_with_resource(RESOURCE_ID_RSS_NEWS_LOGO);
    }
}

static GBitmap* getImage(char* source) {
  ++imageNumber;
  if (imageNumber > 3) {
    imageNumber = 1;
  }

  GBitmap* image = NULL;
  if (imageNumber == 1) {
    image = getImageIfForSource(source);
    if (image != NULL) {
        gbitmap_destroy(bbc_image1);
        bbc_image1 = image;
    }
  } else if (imageNumber == 2) {
    image = getImageIfForSource(source);
    if (image != NULL) {
        gbitmap_destroy(bbc_image2);
        bbc_image2 = image;
    }
  } else if (imageNumber == 3) {
    image = getImageIfForSource(source);
    if (image != NULL) {
        gbitmap_destroy(bbc_image3);
        bbc_image3 = image;
    }
  }
  return image;
}

#ifndef PBL_PLATFORM_APLITE
static void menu_draw_row_callback_basalt(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  GBitmap* image = getImage(_latestSource[cell_index->row]);
  if (image != NULL) {
#ifdef PBL_ROUND
    GRect bounds = GRect(25, 8, 48, 48);
#else
    GRect bounds = GRect(5, 8, 48, 48);
#endif
    graphics_draw_bitmap_in_rect(ctx, image, bounds);
  }

    graphics_context_set_text_color(ctx, GColorLightGray);
#ifdef PBL_ROUND
    GRect source_bounds = GRect(79, 8, 90, 20);
#else
    GRect source_bounds = GRect(59, 8, 90, 20);
#endif
    graphics_draw_text(ctx, _latestSource[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), source_bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

    graphics_context_set_text_color(ctx, GColorVividCerulean);
#ifdef PBL_ROUND
    GRect cat_bounds = GRect(79, 37, 90, 10);
#else
    GRect cat_bounds = GRect(59, 37, 90, 10);
#endif
    graphics_draw_text(ctx, _latestCategory[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), cat_bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);

    if (selectedMenuCell == cell_index->row) {
        graphics_context_set_text_color(ctx, GColorWhite);
    } else {
        graphics_context_set_text_color(ctx, GColorBlack);
    }
#ifdef PBL_ROUND
    GRect headline_bounds = GRect(25, 59, 136, row_height(cell_index));
#else
    GRect headline_bounds = GRect(5, 59, 136, row_height(cell_index));
#endif
    graphics_draw_text(ctx, _latest[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), headline_bounds, GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
}
#else
static void menu_draw_row_callback_aplite(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    GBitmap* image = getImage(_latestSource[cell_index->row]);
    if (image != NULL) {
        GRect bounds = GRect(5, 8, 48, 48);
        graphics_draw_bitmap_in_rect(ctx, image, bounds);
    }

    if (selectedMenuCell == cell_index->row) {
        graphics_context_set_text_color(ctx, GColorWhite);
    } else {
        graphics_context_set_text_color(ctx, GColorBlack);
    }
    graphics_draw_text(ctx, _latestSource[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(59, 8, 90, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, _latestCategory[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(59, 37, 90, 10), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, _latest[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_14), GRect(5, 59, 136, row_height(cell_index)), GTextOverflowModeWordWrap, GTextAlignmentLeft, NULL);
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

static void do_post(char *url) {
    if (bluetooth_connection_service_peek()) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Phone is connected!");

      DictionaryIterator *iter;
      app_message_outbox_begin(&iter);
      dict_write_cstring(iter, READING_LIST, url);
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
  menu_layer_destroy(s_menu_layer);
  text_layer_destroy(s_textlayer_1);
  text_layer_destroy(s_textlayer_2);

  gbitmap_destroy(bbc_image1);
  gbitmap_destroy(bbc_image2);
  gbitmap_destroy(bbc_image3);

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

static void handle_window_unload(Window* window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "unload");
  destroy_ui();
}

static void handle_window_appear(Window* window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "appear");
  hide_animation();
}

static void handle_window_disappear(Window* window) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "disappear");
  hide_latest_view();
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
//#ifdef PBL_PLATFORM_APLITE
//  window_set_fullscreen(s_window, true);
//#endif

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = handle_window_load,
    .unload = handle_window_unload,
    .appear = handle_window_appear,
    .disappear = handle_window_disappear,
  });

  window_set_click_config_provider(s_window, (ClickConfigProvider) config_provider);

  Layer *window_layer = window_get_root_layer(s_window);

#ifdef PBL_ROUND
  s_menu_layer = menu_layer_create(GRect(0, 0, 180, 180));
#else
  s_menu_layer = menu_layer_create(GRect(0, 0, 144, 152));
#endif
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
  
#ifndef PBL_PLATFORM_APLITE
  // Set up the status bar last to ensure it is on top of other Layers
  s_status_bar = status_bar_layer_create();
  layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));

#ifdef PBL_ROUND
  s_bitmap_layer = bitmap_layer_create(GRect(57, 52, 64, 64));
#else
  s_bitmap_layer = bitmap_layer_create(GRect(37, 52, 64, 64));
#endif
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

#ifdef PBL_ROUND
  s_textlayer_1 = text_layer_create(GRect(0, 52, 180, 27));
#else
  s_textlayer_1 = text_layer_create(GRect(0, 52, 144, 27));
#endif
  text_layer_set_text(s_textlayer_1, "Currently unavailable");
  text_layer_set_text_alignment(s_textlayer_1, GTextAlignmentCenter);
  text_layer_set_font(s_textlayer_1, s_res_droid_serif_28_bold);
  layer_insert_above_sibling((Layer *)s_textlayer_1, (Layer *)s_menu_layer);
  hide_no_con_error();

  bbc_image1 = gbitmap_create_with_resource(RESOURCE_ID_BBC_LOGO);
  bbc_image2 = gbitmap_create_with_resource(RESOURCE_ID_BBC_LOGO);
  bbc_image3 = gbitmap_create_with_resource(RESOURCE_ID_BBC_LOGO);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Initialised latest view ui");
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  hide_animation();
}

void show_latest_view(char **latest, char **latestUrl, char **latestSource, char **latestCategory, int num) {
  selectedMenuCell = 0;
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