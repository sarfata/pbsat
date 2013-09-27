#include <pebble_os.h>
#include <pebble_app.h>
#include <pebble_fonts.h>

#define KEY_RISETIME 0
#define KEY_DURATION 1
#define KEY_TIMEZONE_OFFSET 12

static Window *window;

static TextLayer *time_legend, *utc_legend, *countdown_legend;
static TextLayer *time_layer;
static TextLayer *utc_layer;
static TextLayer *countdown_layer;

static char time_str[20] = "";
static char utc_str[20] = "";
static char countdown_str[20] = "";
static int time_delta = 0;

static bool announced_pass = false;
static time_t pass_start, pass_end;

TextLayer *configure_legend_layer(TextLayer *layer, char *initial_text) {
  text_layer_set_background_color(layer, GColorBlack);
  text_layer_set_text_color(layer, GColorWhite);
  text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
  text_layer_set_text(layer, initial_text);
  return layer;
}

TextLayer *configure_time_layer(TextLayer *layer) {
  text_layer_set_background_color(layer, GColorWhite);
  text_layer_set_text_color(layer, GColorBlack);
  text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(layer, GTextAlignmentCenter);
  return layer;
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_t local, utc, diff;

  time(&local);
  utc = local + time_delta;

  strftime(time_str, 20, "%H:%M:%S", localtime(&local));
  strftime(utc_str, 20, "%H:%M:%S",  gmtime(&utc));

  text_layer_set_text(time_layer, time_str);
  text_layer_set_text(utc_layer, utc_str);

  if (utc < pass_start) {
    diff = pass_start - utc;
    if (diff < 120 && announced_pass == false) {
      //vibes_enqueue_custom_pattern(iss_pattern);
    }
    strftime(countdown_str, 20, "%H:%M:%S", gmtime(&diff));
  }
  else if (utc < pass_end) {
    diff = pass_end - utc;
    strftime(countdown_str, 20, "^ %H:%M:%S ^", gmtime(&diff));
  }
  else {
    snprintf(countdown_str, 20, "--:--:--");
  }
  text_layer_set_text(countdown_layer, countdown_str);
}

void appmsg_out_sent(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Out sent.");
}

void appmsg_out_failed(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Out failed: %i", reason);
}

void appmsg_in_received(DictionaryIterator *received, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In received.");

  Tuple *risetime_tuple = dict_find(received, KEY_RISETIME);
  Tuple *duration_tuple = dict_find(received, KEY_DURATION);
  Tuple *timezone_offset_tuple = dict_find(received, KEY_TIMEZONE_OFFSET);

  if (risetime_tuple) {
    int risetime = atoi(risetime_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "Got risetime: %i", risetime);
    if (risetime != pass_start) {
      pass_start = risetime;
      announced_pass = false;
    }
  }
  if (duration_tuple) {
    int duration = atoi(duration_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "Got duration: %i", duration);
    pass_end = pass_start + duration;
  }
  if (timezone_offset_tuple) {
    int timezone_offset = atoi(timezone_offset_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "Got timezone offset: %i", timezone_offset);
    time_delta = timezone_offset;
  }
}

void appmsg_in_dropped(void *context, AppMessageResult reason) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In dropped: %i", reason);
}

void appmsg_out_next(AppMessageResult result, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Out next.");
}


struct AppMessageCallbacksNode callbacksNode = {
  .callbacks = {
    .out_sent = appmsg_out_sent,
    .out_failed = appmsg_out_failed,
    .in_received = appmsg_in_received,
    .in_dropped = appmsg_in_dropped,
    .out_next = appmsg_out_next
  }
};

void handle_init(void) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Pebble Sat starting!");
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_fullscreen(window, true);
  window_set_background_color(window, GColorBlack);

  time_legend = configure_legend_layer(text_layer_create(GRect(0, 10, 144, 20)), "Local Time");
  time_layer = configure_time_layer(text_layer_create(GRect(0, 30, 144, 20)));
  utc_legend = configure_legend_layer(text_layer_create(GRect(0, 70, 144, 20)), "UTC Time");
  utc_layer =  configure_time_layer(text_layer_create(GRect(0, 90, 144, 20)));
  countdown_legend = configure_legend_layer(text_layer_create(GRect(0, 130, 144, 20)), "Next ISS Pass");
  countdown_layer = configure_time_layer(text_layer_create(GRect(0, 150, 144, 20)));

  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_legend));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(utc_legend));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(utc_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(countdown_legend));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(countdown_layer));

  app_message_open(32, 32);
  app_message_register_callbacks(&callbacksNode);

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();

  text_layer_destroy(countdown_legend);
  text_layer_destroy(time_legend);
  text_layer_destroy(utc_legend);

  text_layer_destroy(time_layer);
  text_layer_destroy(utc_layer);
  text_layer_destroy(countdown_layer);

  window_destroy(window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
