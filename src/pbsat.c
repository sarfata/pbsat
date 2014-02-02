#include <pebble.h>

#include "pbsat.h"
#include "iss_ui.h"
#include "pass_ui.h"
#include "comm.h"

static struct PebbleSat {
  ISSUI *iss_ui;
  PassUI *pass_ui;
  ISSData *iss_data;
  bool pass_view;
} pbsat;

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  time_t local, utc, diff;

  time(&local);

  if (!pbsat.iss_data->got_error) {
    utc = local + pbsat.iss_data->time_delta;
    diff = pbsat.iss_data->pass_start - utc;

    if (pbsat.iss_data->pass_start == 0) {
      // If we do not have an error - and do not have a next pass, we are basically loading...
      update_iss_ui(pbsat.iss_ui, local, 0, NULL);
    }
    else if (utc >= pbsat.iss_data->pass_start && utc < pbsat.iss_data->pass_end) {
      // Pass in progress. Show (if needed) and update the PassWindow
      if (pbsat.pass_ui == NULL) {
        pbsat.pass_ui = init_pass_ui();
      }
      update_pass_ui(pbsat.pass_ui, pbsat.iss_data);
    }
    else {
      // Normal view.
      if (pbsat.pass_ui != NULL) {
        // Remove the pass view if it is still there
        window_stack_pop(true);
        destroy_pass_ui(pbsat.pass_ui);
        pbsat.pass_ui = NULL;
        request_update();
      }
      update_iss_ui(pbsat.iss_ui, local, diff, NULL);
    }
  }
  else {
    update_iss_ui(pbsat.iss_ui, local, 0, pbsat.iss_data->error_msg);
  }
}

void handle_init(void) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Pebble Sat starting! v1.0.5");

  pbsat.iss_data = malloc(sizeof(ISSData));

  pbsat.iss_ui = init_iss_ui();
  init_comm(pbsat.iss_data);

  pbsat.pass_view = false;

  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  deinit_comm();
  if (pbsat.pass_ui) {
    destroy_pass_ui(pbsat.pass_ui);
  }
  deinit_iss_ui(pbsat.iss_ui);
  sp_list_free(pbsat.iss_data->position_list);
  free(pbsat.iss_data);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
