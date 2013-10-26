#include <pebble_os.h>
#include <pebble_fonts.h>
#include "pbsat.h"


/* Forward declarations of load and unload */
void iss_load(Window *);
void iss_unload(Window *);

static WindowHandlers window_handlers = {
  .load = &iss_load,
  .unload = &iss_unload
};

ISSUI *init_ui() {
  ISSUI *iss_ui = malloc(sizeof(ISSUI));
  APP_LOG(APP_LOG_LEVEL_INFO, "Init UI()");

  iss_ui->window = window_create();
  window_set_fullscreen(iss_ui->window, true);
  window_set_background_color(iss_ui->window, GColorBlack);
  window_set_window_handlers(iss_ui->window, window_handlers);
  window_set_user_data(iss_ui->window, iss_ui);

  window_stack_push(iss_ui->window, true /* Animated */);

  return iss_ui;
}

void deinit_ui(ISSUI *iss_ui) {
  window_destroy(iss_ui->window);
  free(iss_ui);
}

static TextLayer *configure_legend_layer(TextLayer *layer, char *initial_text) {
  text_layer_set_background_color(layer, GColorBlack);
  text_layer_set_text_color(layer, GColorWhite);
  text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
  text_layer_set_text(layer, initial_text);
  return layer;
}

static TextLayer *configure_time_layer(TextLayer *layer) {
  text_layer_set_background_color(layer, GColorWhite);
  text_layer_set_text_color(layer, GColorBlack);
  text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
  return layer;
}


void iss_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Loading window...");
  ISSUI *iss_ui = window_get_user_data(window);

  iss_ui->time_legend = configure_legend_layer(text_layer_create(GRect(0, 10, 144, 20)), "Local Time");
  iss_ui->time_layer = configure_time_layer(text_layer_create(GRect(0, 30, 144, 20)));
  iss_ui->utc_legend = configure_legend_layer(text_layer_create(GRect(0, 70, 144, 20)), "UTC Time");
  iss_ui->utc_layer =  configure_time_layer(text_layer_create(GRect(0, 90, 144, 20)));
  iss_ui->countdown_legend = configure_legend_layer(text_layer_create(GRect(0, 130, 144, 20)), "Next ISS Pass");
  iss_ui->countdown_layer = configure_time_layer(text_layer_create(GRect(0, 150, 144, 20)));

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(iss_ui->time_legend));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(iss_ui->time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(iss_ui->utc_legend));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(iss_ui->utc_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(iss_ui->countdown_legend));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(iss_ui->countdown_layer));

}

void iss_unload(Window* window) {
  ISSUI *iss_ui = window_get_user_data(window);

  text_layer_destroy(iss_ui->countdown_legend);
  text_layer_destroy(iss_ui->time_legend);
  text_layer_destroy(iss_ui->utc_legend);

  text_layer_destroy(iss_ui->time_layer);
  text_layer_destroy(iss_ui->utc_layer);
  text_layer_destroy(iss_ui->countdown_layer);
}
