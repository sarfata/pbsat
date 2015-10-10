#pragma once

#include <pebble.h>

typedef struct {
  Layer *layer;
  GBitmap *bitmap;
  int32_t center_longitude;
  
} MapLayer;

MapLayer *map_layer_create(GRect bounds, uint32_t resource_id);

void map_layer_set_center_longitude(MapLayer *map_layer, int32_t longitude);

Layer *map_layer_get_layer(MapLayer *map_layer);

void map_layer_destroy(MapLayer *map_layer);

void map_layer_update_proc(Layer *layer, GContext *ctx);
