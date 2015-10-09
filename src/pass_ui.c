#include <pebble.h>
#include "pbsat.h"
#include "pass_ui.h"

typedef struct {
  SkyPosition *position_list;
  int azimuth;
  int elevation;
} SkymapData;

/* Forward declarations */
void pass_load(Window *);
void pass_unload(Window *);
void skymap_draw(Layer *, GContext *ctx);

static void skymap_interpolate_current_position(ISSData *iss_data, SkymapData *skymap_data);
GPoint convert_azel_to_pixels(Layer *l, int az, int el);
int interpolate(int xMax, int y1, int y2, int x);


static WindowHandlers window_handlers = {
  .load = &pass_load,
  .unload = &pass_unload
};

PassUI *init_pass_ui() {
  PassUI *pass_ui = malloc(sizeof(PassUI));
  APP_LOG(APP_LOG_LEVEL_INFO, "Init PassUI");

  pass_ui->window = window_create();
#ifdef PBL_SDK_2
  window_set_fullscreen(pass_ui->window, true);
#endif
  window_set_background_color(pass_ui->window, GColorBlack);
  window_set_window_handlers(pass_ui->window, window_handlers);
  window_set_user_data(pass_ui->window, pass_ui);

  window_stack_push(pass_ui->window, false);

  return pass_ui;
}

void destroy_pass_ui(PassUI *pass_ui) {
  window_destroy(pass_ui->window);
  free(pass_ui);
}

void update_pass_ui(PassUI* pass_ui, ISSData *iss_data)
{
  SkymapData *skymap_data = layer_get_data(pass_ui->skymap_layer);
  skymap_data->position_list = iss_data->position_list;
  skymap_interpolate_current_position(iss_data, skymap_data);

  layer_mark_dirty(pass_ui->skymap_layer);
}

void pass_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Loading PassUI window...");
  PassUI *pass_ui = window_get_user_data(window);

#ifdef PBL_SDK_3
  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  GRect skymap_rect = grect_inset(bounds, GEdgeInsets(18, 0, 15));
#else
  GRect skymap_rect = GRect(0, 15, 144, 144);
#endif
  pass_ui->skymap_layer = layer_create_with_data(skymap_rect, sizeof(SkymapData));
  layer_set_update_proc(pass_ui->skymap_layer, skymap_draw);
  layer_set_clips(pass_ui->skymap_layer, false);
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

  layer_destroy(pass_ui->skymap_layer);
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

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "AZEL: %i / %i - maps to pixels: %i / %i", az, el, p.x, p.y);
  return p;
}

// Simple linear interpolation
int interpolate(int xMax, int y1, int y2, int x) {
  int m = (y2 - y1) * 1000 / xMax;
  return y1 + (x * m) / 1000;
}

static void skymap_interpolate_current_position(ISSData *iss_data, SkymapData *skymap_data) {
  time_t local, utc;
  time(&local);
  utc = local + iss_data->time_delta;

  SkyPosition *pos = skymap_data->position_list;

  // We need at least two segments
  if (pos == NULL || pos->next == NULL)
    return;

  // Dont draw if we are before the first segment
  if (utc < pos->timestamp)
    return;

  // Find the segment of positions we are on.
  while (pos->next && utc > pos->next->timestamp) {
    pos = pos->next;
  }

  // Dont draw if we are already passed the end of the segment
  if (utc > pos->next->timestamp)
    return;

  // Simple interpolation of az and el.
  int az = interpolate(pos->next->timestamp - pos->timestamp, pos->azimuth, pos->next->azimuth, utc - pos->timestamp);
  int el = interpolate(pos->next->timestamp - pos->timestamp, pos->elevation, pos->next->elevation, utc - pos->timestamp);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Position %i (%i / %i) %i (%i / %i) at %li - %li%% (%li / %li)",
    az, pos->azimuth, pos->next->azimuth,
    el, pos->elevation, pos->next->elevation,
    utc,
    (utc - pos->timestamp) * 100 / (pos->next->timestamp - pos->timestamp),
    pos->timestamp, pos->next->timestamp);

  skymap_data->azimuth = az;
  skymap_data->elevation = el;
}

#ifdef PBL_SDK_3

void skymap_draw_cadran(Layer *l, GContext *ctx) {
  GRect bounds = layer_get_bounds(l);

  // Draw a large circle for horizon
  graphics_context_set_stroke_color(ctx, GColorCobaltBlue);
  graphics_context_set_stroke_width(ctx, 5);
  graphics_draw_arc(ctx, bounds, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(359));

  // Draw a lighter circle for 30º level
  graphics_context_set_stroke_width(ctx, 2);
  int inset1 = bounds.size.w / 2 / 3;
  GRect rect1 = grect_inset(bounds, GEdgeInsets(inset1));
  graphics_draw_arc(ctx, rect1, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(359));

  // Draw a lighter circle for 60º level
  graphics_context_set_stroke_width(ctx, 1);
  int inset2 = inset1 * 2;
  GRect rect2 = grect_inset(bounds, GEdgeInsets(inset2));
  graphics_draw_arc(ctx, rect2, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(359));

  // Draw lines of degrading width
  for (int angle = 0; angle  <= 360; angle += 90) {
    GPoint a = gpoint_from_polar(bounds, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(angle));
    GPoint b = gpoint_from_polar(rect1, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(angle));
    graphics_context_set_stroke_width(ctx, 5);
    graphics_draw_line(ctx, a, b);

    GPoint c = gpoint_from_polar(rect2, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(angle));
    graphics_context_set_stroke_width(ctx, 2);
    graphics_draw_line(ctx, b, c);
  }
}

#else

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

#endif


void skymap_draw_trajectory(Layer *l, GContext *ctx, SkyPosition *pos) {
#ifdef PBL_SDK_3
  graphics_context_set_stroke_width(ctx, 2);
#endif
  graphics_context_set_stroke_color(ctx, PBL_IF_COLOR_ELSE(GColorIcterine, GColorBlack));

  // We need at least two segments
  if (pos == NULL || pos->next == NULL)
    return;

  while (pos->next) {
    GPoint a = convert_azel_to_pixels(l, pos->azimuth, pos->elevation);
    GPoint b = convert_azel_to_pixels(l, pos->next->azimuth, pos->next->elevation);
    graphics_draw_line(ctx, a, b);
    pos = pos->next;
  }
}


void skymap_draw_position(Layer *l, GContext *ctx, int az, int el) {
  graphics_context_set_fill_color(ctx, GColorWhite);

  GPoint p = convert_azel_to_pixels(l, az, el);

  graphics_fill_circle(ctx, p, 3);
}

void skymap_draw(Layer *l, GContext *ctx) {
  SkymapData *skymap_data = layer_get_data(l);

  skymap_draw_cadran(l, ctx);
  skymap_draw_trajectory(l, ctx, skymap_data->position_list);
  skymap_draw_position(l, ctx, skymap_data->azimuth, skymap_data->elevation);
}
