#include <pebble.h>

/* Define a structure to hold of our UI elements */
typedef struct {
  Window *window;
  GBitmap *iss_bmp;
  GBitmap *pass_bmp;

  BitmapLayer *background_layer;

  Layer *pass_layer;
  BitmapLayer *pass_icon_layer;
  TextLayer *pass_text_layer;

  TextLayer *error_layer;

  InverterLayer *time_bg_layer;
  TextLayer *time_layer;

  /* Strings to hold the text that will be displayed in the layers */
  char error_str[42];
  char pass_str[10];
  char time_str[6];
} ISSUI;

/* And some functions to manipulate it */
ISSUI *init_iss_ui();
void deinit_iss_ui(ISSUI*);

void update_iss_ui(ISSUI*, time_t time, time_t next_pass, char *error);
