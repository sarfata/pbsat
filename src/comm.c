#include <pebble.h>
#include "pbsat.h"
#include "comm.h"

static void clear_error(ISSData* iss_data) {
  iss_data->got_error = false;
}

static void set_error(ISSData* iss_data, char *error) {
  strncpy(iss_data->error_msg, error, sizeof(iss_data->error_msg));
  iss_data->got_error = true;
}

static void appmsg_in_received(DictionaryIterator *received, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In received.");

  ISSData *iss_data = (ISSData *) context;

  Tuple *risetime_tuple = dict_find(received, KEY_RISETIME);
  Tuple *duration_tuple = dict_find(received, KEY_DURATION);
  Tuple *timezone_offset_tuple = dict_find(received, KEY_TIMEZONE_OFFSET);
  Tuple *error_tuple = dict_find(received, KEY_ERROR);

  if (risetime_tuple) {
    int32_t risetime = risetime_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "Got risetime: %li", risetime);
    if (risetime != iss_data->pass_start) {
      iss_data->pass_start = risetime;
      iss_data->announced_pass = false;
    }
    clear_error(iss_data);
  }
  if (duration_tuple) {
    int32_t duration = duration_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "Got duration: %li", duration);
    iss_data->pass_end = iss_data->pass_start + duration;
    clear_error(iss_data);
  }
  if (timezone_offset_tuple) {
    int32_t timezone_offset = timezone_offset_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "Got timezone offset: %li", timezone_offset);
    iss_data->time_delta = timezone_offset;
    iss_data->got_time_delta = true;
  }
  if (error_tuple) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Got error: %s", error_tuple->value->cstring);
    set_error(iss_data, error_tuple->value->cstring);
  }
}

static void appmsg_in_dropped(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In dropped: %i", reason);
  // Request a new update...
  request_update();
}

static void appmsg_out_sent(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Out sent.");
}

static void appmsg_out_failed(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  ISSData *iss_data = (ISSData *) context;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Out failed: %i", reason);

  switch (reason) {
    case APP_MSG_SEND_REJECTED:
    case APP_MSG_SEND_TIMEOUT:
      set_error(iss_data, "Phone not responding. Please reinstall.");
      request_update();
      break;
    case APP_MSG_NOT_CONNECTED:
      set_error(iss_data, "Phone not connected");
      request_update();
      break;
    default:
      // For all other errors. Just retry.
      request_update();
      break;
  }
}

// Ask the JS code to send a new update
void request_update() {
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_uint8(iter, KEY_REQUEST_UPDATE, 42);

  app_message_outbox_send();
}

void init_comm(ISSData *iss_data) {
  clear_error(iss_data);

  app_message_register_inbox_received(appmsg_in_received);
  app_message_register_inbox_dropped(appmsg_in_dropped);
  app_message_register_outbox_sent(appmsg_out_sent);
  app_message_register_outbox_failed(appmsg_out_failed);
  app_message_set_context(iss_data);

  app_message_open(124, 124);

  request_update();
}

void deinit_comm() {
  app_message_deregister_callbacks();
}
