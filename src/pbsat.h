#include <pebble.h>

/* Define the keys that will be used in AppMessage communication.
 * They must match what you define in appinfo.json -> appKeys.
 */
#define KEY_RISETIME        420
#define KEY_DURATION        421
#define KEY_TIMEZONE_OFFSET 422
#define KEY_ERROR           423
#define KEY_REQUEST_UPDATE  424

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

/* Define a structure that defines the current status of the ISS */
typedef struct {
  /* Time (UTC) when the next pass will start */
  time_t pass_start;
  /* Time (UTC) when the next pass will end */
  time_t pass_end;
  /* Have we warned the user that this pass is imminent? */
  bool announced_pass;
  /* Difference between localtime and UTC time in seconds */
  int32_t time_delta;
} ISSData;

ISSUI *init_ui();
void deinit_ui(ISSUI*);

void init_comm(ISSData *);
void deinit_comm();
