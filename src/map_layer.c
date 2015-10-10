#include "map_layer.h"

MapLayer *map_layer_create(GRect bounds, uint32_t resource_id) {
  MapLayer *map_layer = malloc(sizeof(MapLayer));
  if (!map_layer) {
    return map_layer;
  }

  map_layer->bitmap = gbitmap_create_with_resource(resource_id);
  if (!map_layer->bitmap) {
    free(map_layer);
    return NULL;
  }

  map_layer->layer = layer_create_with_data(bounds, sizeof(MapLayer*));
  MapLayer **ptr = layer_get_data(map_layer->layer);
  *ptr = map_layer;
  layer_set_update_proc(map_layer->layer, map_layer_update_proc);

  // TODO: measure longitude in degrees and not pixels
  map_layer->center_longitude = DEG_TO_TRIGANGLE(-179);
  //map_layer->center_longitude = 401;

  return map_layer;
}

Layer *map_layer_get_layer(MapLayer *map_layer) {
  return map_layer->layer;
}

void map_layer_destroy(MapLayer *map_layer) {
  gbitmap_destroy(map_layer->bitmap);
  layer_destroy(map_layer->layer);
  free(map_layer);
}

void map_layer_set_center_longitude(MapLayer *map_layer, int32_t center_longitude) {
  map_layer->center_longitude = center_longitude;
  layer_mark_dirty(map_layer->layer);
}

const int MAP_OFFSET_IN_DEGREES = 180;

// With DRAW_MAP_WITH_TWO_CALLS enabled we will draw the map twice to fill the entire screen (only when needed).
// When it is not enabled we will use one draw bitmap call and relie on the bitmap "pattern" feature. This is the cleaner
// solution but seems to leave an "artifact" on the screen unfortunately so it is not enabled now.
//#define DRAW_MAP_WITH_TWO_CALLS

void map_layer_update_proc(Layer *layer, GContext *ctx) {
  MapLayer  *map_layer = *((MapLayer**)layer_get_data(layer));

  GRect layer_bounds = layer_get_frame(layer);
  GRect map_bounds = gbitmap_get_bounds(map_layer->bitmap);

  int32_t angle_from_left_of_map = ((MAP_OFFSET_IN_DEGREES * TRIG_MAX_ANGLE / 360) + map_layer->center_longitude);
  angle_from_left_of_map = angle_from_left_of_map % TRIG_MAX_ANGLE;
  // Details of the maths - (This is not dead code ;)
  // center_pixel = width / 360 * angle_from_left_of_map_degrees
  // angle_degrees = angle_from_left_of_map * 360 / TRIG_MAX_ANGLE
  // center_pixel = width / 360 * angle_from_left_of_map * 360 / TRIG_MAX_ANGLE
  // center_pixel = width * angle_from_left_of_map / TRIG_MAX_ANGLE
  int center_pixel = angle_from_left_of_map * map_bounds.size.w / TRIG_MAX_ANGLE;
  int left_corner =  center_pixel - layer_bounds.size.w / 2;

  // Normalize left_corner to 0 -> map_bounds.size.w
  left_corner = (left_corner + map_bounds.size.w) % map_bounds.size.w;

  //printf("Center longitude=%0lx Angle from left of map=%0lx center_pixel=%d left_corner=%d",
  //    map_layer->center_longitude, angle_from_left_of_map, center_pixel, left_corner);

  map_bounds.origin.x = -left_corner;

#ifndef DRAW_MAP_WITH_TWO_CALLS
  map_bounds.size.w = layer_bounds.size.w + left_corner;
#endif
  //printf("Drawing1 the map %d %d %d %d", map_bounds.origin.x, map_bounds.origin.y, map_bounds.size.w, map_bounds.size.h);
  graphics_draw_bitmap_in_rect(ctx, map_layer->bitmap, map_bounds);

#ifdef DRAW_MAP_WITH_TWO_CALLS
  map_bounds.origin.x += map_bounds.size.w;
  map_bounds.size.w = layer_bounds.size.w - map_bounds.origin.x;
  if (map_bounds.size.w > 0) {
    graphics_draw_bitmap_in_rect(ctx, map_layer->bitmap, map_bounds);
  }
#endif
}

