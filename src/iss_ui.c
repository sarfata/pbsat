#include <pebble.h>
#include "pbsat.h"
#include "iss_ui.h"

/* Forward declarations of load and unload */
void iss_load(Window *);
void iss_unload(Window *);

static WindowHandlers window_handlers = {
  .load = &iss_load,
  .unload = &iss_unload
};

ISSUI *init_iss_ui() {
  ISSUI *iss_ui = malloc(sizeof(ISSUI));
  APP_LOG(APP_LOG_LEVEL_INFO, "Init UI()");

  iss_ui->window = window_create();
#ifdef PBL_SDK_2
  window_set_fullscreen(iss_ui->window, true);
#endif
  window_set_background_color(iss_ui->window, PBL_IF_COLOR_ELSE(GColorBlueMoon, GColorBlack));
  window_set_window_handlers(iss_ui->window, window_handlers);
  window_set_user_data(iss_ui->window, iss_ui);

  window_stack_push(iss_ui->window, true /* Animated */);

  return iss_ui;
}

void deinit_iss_ui(ISSUI *iss_ui) {
  window_destroy(iss_ui->window);
  free(iss_ui);
}

void update_iss_ui(ISSUI* iss_ui, time_t local, time_t countdown, char *error)
{
#ifdef PBL_SDK_3
  //map_layer_set_center_longitude(iss_ui->world_map, (local*3 % 360) * TRIG_MAX_ANGLE / 360);
#else
  // Localtime is always a valid info.
  strftime(iss_ui->time_str, sizeof(iss_ui->time_str), "%H:%M", localtime(&local));
  text_layer_set_text(iss_ui->time_layer, iss_ui->time_str);

  // Only display pass info if there is no error.
  if (!error) {
    bitmap_layer_set_bitmap(iss_ui->background_layer, iss_ui->iss_bmp);

    layer_set_hidden(text_layer_get_layer(iss_ui->error_layer), true);
    layer_set_hidden(iss_ui->pass_layer, false);

    if (countdown > 0) {
      strftime(iss_ui->pass_str, sizeof(iss_ui->pass_str), "%H:%M:%S", gmtime(&countdown));
    }
    else {
      strncpy(iss_ui->pass_str, "...", sizeof(iss_ui->pass_str));
    }
    text_layer_set_text(iss_ui->pass_text_layer, iss_ui->pass_str);
  }
  else {
    bitmap_layer_set_bitmap(iss_ui->background_layer, iss_ui->error_bmp);

    layer_set_hidden(text_layer_get_layer(iss_ui->error_layer), false);
    layer_set_hidden(iss_ui->pass_layer, true);

    strncpy(iss_ui->error_str, error, sizeof(iss_ui->error_str));
    text_layer_set_text(iss_ui->error_layer, iss_ui->error_str);
  }
#endif
}


#ifdef PBL_SDK_3

void animate_update(Animation *animation, const AnimationProgress progress) {
  MapLayer *map_layer = animation_get_context(animation);
  // progress goes from ANIM_MIN to ANIM_MAX
  // degrees goes from 0 to 360
  // progress_pct = (progress-ANIN_MIN) / (ANIM_MAX-ANIM_MIN);
  // progress_deg = progress_pct * TRIG_ANGLE_MAX / 360;

  int32_t degrees = (progress-ANIMATION_NORMALIZED_MIN) * TRIG_MAX_ANGLE / (ANIMATION_NORMALIZED_MAX -ANIMATION_NORMALIZED_MIN);
  degrees %= 40 * (TRIG_MAX_ANGLE / 360);
  degrees = -140 * TRIG_MAX_ANGLE / 360 + degrees;
  map_layer_set_center_longitude(map_layer, degrees);
}

void iss_load(Window *window) {
  ISSUI *iss_ui = window_get_user_data(window);
  GRect bounds = layer_get_bounds(window_get_root_layer(window));

  iss_ui->world_map = map_layer_create(bounds, RESOURCE_ID_WORLD_MAP);
  layer_add_child(window_get_root_layer(window), map_layer_get_layer(iss_ui->world_map));

  Animation *animation = animation_create();
  animation_set_duration(animation, 30000);
  animation_set_play_count(animation, ANIMATION_PLAY_COUNT_INFINITE);
  animation_set_curve(animation, AnimationCurveLinear);
  animation_set_handlers(animation, (AnimationHandlers) { .started = NULL, .stopped = NULL } , iss_ui->world_map);

  static AnimationImplementation impl = {
    .update = animate_update
  };

  animation_set_implementation(animation, &impl);
  animation_schedule(animation);

}

#else

void iss_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Loading window...");
  ISSUI *iss_ui = window_get_user_data(window);

  iss_ui->iss_bmp = gbitmap_create_with_resource(RESOURCE_ID_ISS_IMAGE);
  iss_ui->pass_bmp = gbitmap_create_with_resource(RESOURCE_ID_PASS_ICON);
  iss_ui->error_bmp = gbitmap_create_with_resource(RESOURCE_ID_ERROR_IMAGE);

  /* Create a layer to hold the ISS (or errro) image */
  iss_ui->background_layer = bitmap_layer_create(GRect(0, 0, 144, 91));
  bitmap_layer_set_alignment(iss_ui->background_layer, GAlignTop);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(iss_ui->background_layer));

  /* Create a layer to hold the pass icon and the pass text */
  iss_ui->pass_layer = layer_create(GRect(0, 92, 144, 20));

  iss_ui->pass_icon_layer = bitmap_layer_create(GRect(0, 0, 42, 20));
  bitmap_layer_set_bitmap(iss_ui->pass_icon_layer, iss_ui->pass_bmp);
  bitmap_layer_set_alignment(iss_ui->pass_icon_layer, GAlignRight);
  layer_add_child(iss_ui->pass_layer, bitmap_layer_get_layer(iss_ui->pass_icon_layer));

  iss_ui->pass_text_layer = text_layer_create(GRect(50, -4, 94, 20));
  text_layer_set_font(iss_ui->pass_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(iss_ui->pass_text_layer, GTextAlignmentLeft);
  text_layer_set_text_color(iss_ui->pass_text_layer, GColorWhite);
  text_layer_set_background_color(iss_ui->pass_text_layer, GColorClear);
  layer_add_child(iss_ui->pass_layer, text_layer_get_layer(iss_ui->pass_text_layer));

  layer_add_child(window_get_root_layer(window), iss_ui->pass_layer);

  /* Create a layer to display errors when needed */
  iss_ui->error_layer = text_layer_create(GRect(0, 50, 144, 62));
  text_layer_set_font(iss_ui->error_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(iss_ui->error_layer, GTextAlignmentCenter);
  text_layer_set_text_color(iss_ui->error_layer, GColorWhite);
  text_layer_set_background_color(iss_ui->error_layer, GColorClear);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(iss_ui->error_layer));

  /* Create a layer to display the time at the bottom of the screen */
  iss_ui->time_layer = text_layer_create(GRect(0, 114, 144, 54));
  text_layer_set_font(iss_ui->time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(iss_ui->time_layer, GTextAlignmentCenter);
  text_layer_set_text_color(iss_ui->time_layer, GColorWhite);
  text_layer_set_background_color(iss_ui->time_layer, GColorBlack);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(iss_ui->time_layer));

  /* Inverse the bottom part of the screen to create contrast around the current time */
  iss_ui->time_bg_layer = inverter_layer_create(GRect(0, 114, 144, 54));
  layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(iss_ui->time_bg_layer));
}
#endif

void iss_unload(Window* window) {
  ISSUI *iss_ui = window_get_user_data(window);

  bitmap_layer_destroy(iss_ui->background_layer);
  bitmap_layer_destroy(iss_ui->pass_icon_layer);

#ifdef PBL_SDK_3
  gbitmap_destroy(iss_ui->world_bmp);
#else
  gbitmap_destroy(iss_ui->error_bmp);
  gbitmap_destroy(iss_ui->iss_bmp);
  gbitmap_destroy(iss_ui->pass_bmp);
#endif
  text_layer_destroy(iss_ui->pass_text_layer);
  layer_destroy(iss_ui->pass_layer);

  text_layer_destroy(iss_ui->error_layer);
  text_layer_destroy(iss_ui->time_layer);
#ifdef PBL_SDK_2
  inverter_layer_destroy(iss_ui->time_bg_layer);
#endif
}
