#include <pebble.h>

#include "pbsat.h"
#include "iss_ui.h"
#include "comm.h"

static struct PebbleSat {
  ISSUI *iss_ui;
  ISSData *iss_data;
} pbsat;

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_t local, utc, diff;

  time(&local);
  utc = local + pbsat.iss_data->time_delta;
  diff = pbsat.iss_data->pass_start - utc;

  if (pbsat.iss_data->pass_start == 0) {
    update_iss_ui(pbsat.iss_ui, local, 0, "No pass");
  }
  else if (utc > pbsat.iss_data->pass_end) {
    update_iss_ui(pbsat.iss_ui, local, 0, "Pass in past");
  }
  else if (utc > pbsat.iss_data->pass_start) {
    update_iss_ui(pbsat.iss_ui, local, 0, "ISS Above!!!");
  }
  else {
    update_iss_ui(pbsat.iss_ui, local, diff, NULL);
  }
}

void handle_init(void) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Pebble Sat starting!");

  pbsat.iss_data = malloc(sizeof(ISSData));

  pbsat.iss_ui = init_iss_ui();
  init_comm(pbsat.iss_data);

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  deinit_comm();
  deinit_iss_ui(pbsat.iss_ui);
  free(pbsat.iss_data);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
