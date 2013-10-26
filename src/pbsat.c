#include <pebble_app.h>
#include <pebble_fonts.h>

#include "pbsat.h"

static ISSUI *iss_ui;
static ISSData *iss_data;

/* Hack until we get window_set_user_data() */
void *g_user_data;

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_t local, utc, diff;

  time(&local);
  utc = local + iss_data->time_delta;

  strftime(iss_ui->time_str, 20, "%H:%M:%S", localtime(&local));
  strftime(iss_ui->utc_str, 20, "%H:%M:%S",  gmtime(&utc));

  text_layer_set_text(iss_ui->time_layer, iss_ui->time_str);
  text_layer_set_text(iss_ui->utc_layer, iss_ui->utc_str);

  if (utc < iss_data->pass_start) {
    diff = iss_data->pass_start - utc;
    if (diff < 120 && iss_data->announced_pass == false) {
      //vibes_enqueue_custom_pattern(iss_pattern);
    }
    strftime(iss_ui->countdown_str, 20, "%H:%M:%S", gmtime(&diff));
  }
  else if (utc < iss_data->pass_end) {
    diff = iss_data->pass_end - utc;
    strftime(iss_ui->countdown_str, 20, "^ %H:%M:%S ^", gmtime(&diff));
  }
  else {
    snprintf(iss_ui->countdown_str, 20, "--:--:--");
  }
  text_layer_set_text(iss_ui->countdown_layer, iss_ui->countdown_str);
}

void handle_init(void) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Pebble Sat starting!");

  iss_data = malloc(sizeof(ISSData));

  iss_ui = init_ui();
  init_comm(iss_data);

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  deinit_comm();
  deinit_ui(iss_ui);
  free(iss_data);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
