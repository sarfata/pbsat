#include <pebble.h>

/* Define a structure to hold of our UI elements */
typedef struct {
  Window *window;

  TextLayer *az_layer;
  TextLayer *el_layer;
  TextLayer *countdown_layer;

  GBitmap *down_icon;
  BitmapLayer *down_icon_layer;

  Layer *skymap_layer;
} PassUI;

/* And some functions to manipulate it */
PassUI *init_pass_ui();
void deinit_pass_ui(PassUI*);

void update_pass_ui(PassUI* pass_ui);
