#include <pebble.h>

#include "pbsat.h"

static ISSUI *iss_ui;
static ISSData *iss_data;

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_t local, utc, diff;

  time(&local);
  utc = local + iss_data->time_delta;

  strftime(iss_ui->time_str, sizeof(iss_ui->time_str), "%H:%M", localtime(&local));
  text_layer_set_text(iss_ui->time_layer, iss_ui->time_str);

  if (utc < iss_data->pass_start) {
    diff = iss_data->pass_start - utc;
    strftime(iss_ui->pass_str, sizeof(iss_ui->pass_str), "%H:%M:%S", gmtime(&diff));
  }
  else if (utc < iss_data->pass_end) {
    diff = iss_data->pass_end - utc;
    strftime(iss_ui->pass_str, sizeof(iss_ui->pass_str), "^ %H:%M:%S ^", gmtime(&diff));
  }
  else {
    snprintf(iss_ui->pass_str, sizeof(iss_ui->pass_str), "--:--:--");
  }
  text_layer_set_text(iss_ui->pass_text_layer, iss_ui->pass_str);
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
