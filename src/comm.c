#include "pbsat.h"

static void appmsg_in_received(DictionaryIterator *received, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In received.");

  ISSData *iss_data = (ISSData *) context;

  Tuple *risetime_tuple = dict_find(received, KEY_RISETIME);
  Tuple *duration_tuple = dict_find(received, KEY_DURATION);
  Tuple *timezone_offset_tuple = dict_find(received, KEY_TIMEZONE_OFFSET);
  Tuple *error_tuple = dict_find(received, KEY_ERROR);

  if (risetime_tuple) {
    int risetime = atoi(risetime_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "Got risetime: %i", risetime);
    if (risetime != iss_data->pass_start) {
      iss_data->pass_start = risetime;
      iss_data->announced_pass = false;
    }
  }
  if (duration_tuple) {
    int duration = atoi(duration_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "Got duration: %i", duration);
    iss_data->pass_end = iss_data->pass_start + duration;
  }
  if (timezone_offset_tuple) {
    int timezone_offset = atoi(timezone_offset_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "Got timezone offset: %i", timezone_offset);
    iss_data->time_delta = timezone_offset;
  }
  if (error_tuple) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Got error: %s", error_tuple->value->cstring);
  }
}

static void appmsg_in_dropped(void *context, AppMessageResult reason) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In dropped: %i", reason);
}

static void appmsg_out_sent(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Out sent.");
}

static void appmsg_out_failed(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Out failed: %i", reason);
}

struct AppMessageCallbacksNode callbacksNode = {
  .callbacks = {
    .out_sent = appmsg_out_sent,
    .out_failed = appmsg_out_failed,
    .in_received = appmsg_in_received,
    .in_dropped = appmsg_in_dropped
  }
};

// Ask the JS code to send a new update
void request_update() {
  DictionaryIterator *iter;
  app_message_out_get(&iter);

  dict_write_uint8(iter, KEY_REQUEST_UPDATE, 42);

  app_message_out_send();
  app_message_out_release();
}

void init_comm(ISSData *iss_data) {
  app_message_open(124, 124);
  callbacksNode.context = iss_data;
  app_message_register_callbacks(&callbacksNode);

  request_update();
}

void deinit_comm() {
  app_message_deregister_callbacks(&callbacksNode);
}


