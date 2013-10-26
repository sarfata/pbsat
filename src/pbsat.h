#include <pebble_os.h>

/* Define the keys that will be used in AppMessage communication.
 * They must match what you define in appinfo.json -> appKeys.
 */
#define KEY_RISETIME        420
#define KEY_DURATION        421
#define KEY_TIMEZONE_OFFSET 422
#define KEY_ERROR           423
#define KEY_REQUEST_UPDATE  424

// A disgusting hack until this is in the firmware
extern void *g_user_data;
#define window_set_user_data(w, p) g_user_data = p
#define window_get_user_data(w) g_user_data

/* Define a structure to hold of our UI elements */
typedef struct {
  Window *window;
  GBitmap *iss_bmp;
  BitmapLayer *background_layer;
  TextLayer *time_legend, *utc_legend, *countdown_legend;
  TextLayer *time_layer;
  TextLayer *utc_layer;
  TextLayer *countdown_layer;
  /* Strings to hold the text that will be displayed in the layers */
  char time_str[20];
  char utc_str[20];
  char countdown_str[20];
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
  int time_delta;
} ISSData;

ISSUI *init_ui();
void deinit_ui(ISSUI*);

void init_comm(ISSData *);
void deinit_comm();
