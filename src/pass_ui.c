#include <pebble.h>
#include "pbsat.h"
#include "pass_ui.h"

/* Forward declarations */
void pass_load(Window *);
void pass_unload(Window *);
void skymap_draw(Layer *, GContext *ctx);

static WindowHandlers window_handlers = {
  .load = &pass_load,
  .unload = &pass_unload
};

PassUI *init_pass_ui() {
  PassUI *pass_ui = malloc(sizeof(PassUI));
  APP_LOG(APP_LOG_LEVEL_INFO, "Init PassUI");

  pass_ui->window = window_create();
  window_set_fullscreen(pass_ui->window, true);
  window_set_background_color(pass_ui->window, GColorBlack);
  window_set_window_handlers(pass_ui->window, window_handlers);
  window_set_user_data(pass_ui->window, pass_ui);

  window_stack_push(pass_ui->window, false);

  return pass_ui;
}

void deinit_pass_ui(PassUI *pass_ui) {
  window_destroy(pass_ui->window);
  free(pass_ui);
}

void update_pass_ui(PassUI* pass_ui)
{
  text_layer_set_text(pass_ui->az_layer, "AZ: 184º");
  text_layer_set_text(pass_ui->el_layer, "EL: 45º");
  text_layer_set_text(pass_ui->countdown_layer, "07:45:00");
  layer_mark_dirty(pass_ui->skymap_layer);
}

void pass_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Loading PassUI window...");
  PassUI *pass_ui = window_get_user_data(window);

  pass_ui->az_layer = text_layer_create(GRect(0, 0, 72, 18));
  text_layer_set_font(pass_ui->az_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(pass_ui->az_layer, GTextAlignmentLeft);
  text_layer_set_text_color(pass_ui->az_layer, GColorWhite);
  text_layer_set_background_color(pass_ui->az_layer, GColorClear);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(pass_ui->az_layer));

  pass_ui->el_layer = text_layer_create(GRect(72, 0, 72, 18));
  text_layer_set_font(pass_ui->el_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(pass_ui->el_layer, GTextAlignmentRight);
  text_layer_set_text_color(pass_ui->el_layer, GColorWhite);
  text_layer_set_background_color(pass_ui->el_layer, GColorClear);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(pass_ui->el_layer));

  pass_ui->countdown_layer = text_layer_create(GRect(20, 148, 104, 20));
  text_layer_set_font(pass_ui->countdown_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(pass_ui->countdown_layer, GTextAlignmentCenter);
  text_layer_set_text_color(pass_ui->countdown_layer, GColorWhite);
  text_layer_set_background_color(pass_ui->countdown_layer, GColorClear);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(pass_ui->countdown_layer));

  pass_ui->down_icon = gbitmap_create_with_resource(RESOURCE_ID_DOWN_ICON);
  pass_ui->down_icon_layer = bitmap_layer_create(GRect(10, 152, 15, 15));
  bitmap_layer_set_bitmap(pass_ui->down_icon_layer, pass_ui->down_icon);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(pass_ui->down_icon_layer));

  pass_ui->skymap_layer = layer_create(GRect(0, 15, 144, 144));
  layer_set_update_proc(pass_ui->skymap_layer, skymap_draw);
  layer_add_child(window_get_root_layer(window), pass_ui->skymap_layer);

  // Schedule vibe to let the user know it's time to look at his Pebble! and at the sky!
  #define DIT 200
  #define DAH 400
  #define PAUSE DIT // correct?
  #define SPACE DAH
  static const uint32_t const segments[] = {
    DIT, PAUSE, DIT, SPACE,                 // 'I'
    DIT, PAUSE, DIT, PAUSE, DIT, SPACE,     // 'S'
    DIT, PAUSE, DIT, PAUSE, DIT             // 'S'
  };
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };
  vibes_enqueue_custom_pattern(pat);
}

void pass_unload(Window* window) {
  PassUI *pass_ui = window_get_user_data(window);

  text_layer_destroy(pass_ui->az_layer);
  text_layer_destroy(pass_ui->el_layer);
  text_layer_destroy(pass_ui->countdown_layer);

  bitmap_layer_destroy(pass_ui->down_icon_layer);
  gbitmap_destroy(pass_ui->down_icon);

  layer_destroy(pass_ui->skymap_layer);
}

void skymap_draw_cadran(Layer *l, GContext *ctx) {
  GRect bounds = layer_get_bounds(l);
  GPoint center;
  center.x = bounds.size.w / 2;
  center.y = bounds.size.h / 2;
  int16_t radius = bounds.size.w / 4 - 5;

  graphics_context_set_stroke_color(ctx, GColorWhite);

  // Draw a small and a bigger circle
  graphics_draw_circle(ctx, center, radius);
  graphics_draw_circle(ctx, center, radius * 2);

  // Draw the N/S Axis
  graphics_draw_line(ctx,
    GPoint(center.x, center.y - (radius*2) - 4),
    GPoint(center.x, center.y + (radius*2) + 4));

  // Draw the W/E Axis
  graphics_draw_line(ctx,
    GPoint(center.x - (radius*2) - 4, center.y),
    GPoint(center.x + (radius*2) + 4, center.y));
}

GPoint convert_azel_to_pixels(Layer *l, int az, int el) {
  GRect bounds = layer_get_bounds(l);
  GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  int16_t radius = bounds.size.w / 4 - 5;

  int scale = radius * 2;

  //cos(az) = x / el;
  //sin(az) = y / el;

  // We want the 0 up north and not on the right like on a trig circle:
  az -= 90;

  GPoint p = center;
  p.x += cos_lookup(TRIG_MAX_ANGLE * az / 360) * (90 - el) / 90 * scale / TRIG_MAX_RATIO;
  p.y += sin_lookup(TRIG_MAX_ANGLE * az / 360) * (90 - el) / 90 * scale / TRIG_MAX_RATIO;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "AZEL: %i / %i - maps to pixels: %i / %i", az, el, p.x, p.y);
  return p;
}

static int az = 10;
static int el = 0;

void skymap_draw_position(Layer *l, GContext *ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);

  az = az + 2;
  az = az % 360;

  el = el + 1;
  el = el % 90;

  GPoint p = convert_azel_to_pixels(l, az, el);

  graphics_draw_circle(ctx, p, 3);
}

void skymap_draw(Layer *l, GContext *ctx) {
  skymap_draw_cadran(l, ctx);
  //skymap_draw_trajectory(l, ctx);
  skymap_draw_position(l, ctx);
}
