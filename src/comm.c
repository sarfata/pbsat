#include <pebble.h>
#include "pbsat.h"
#include "comm.h"

static char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}

static SkyPosition *load_positions(DictionaryIterator *received);

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
  Tuple *settime_tuple = dict_find(received, KEY_SETTIME);
  Tuple *timezone_offset_tuple = dict_find(received, KEY_TIMEZONE_OFFSET);
  Tuple *error_tuple = dict_find(received, KEY_ERROR);
  Tuple *positions_tuple = dict_find(received, KEY_POSITIONS_OFFSET);

  if (risetime_tuple && settime_tuple) {
    int32_t risetime = risetime_tuple->value->int32;
    int32_t settime = settime_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "Got new pass - Rise: %li Set: %li", risetime, settime);
    iss_data->pass_start = risetime;
    iss_data->pass_end = settime;
    iss_data->announced_pass = false;
    sp_list_free(iss_data->position_list);
    iss_data->position_list = NULL;
    clear_error(iss_data);
  }
  else if (timezone_offset_tuple) {
    int32_t timezone_offset = timezone_offset_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "Got timezone offset: %li", timezone_offset);
    iss_data->time_delta = timezone_offset;
  }
  else if (error_tuple) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Got error: %s", error_tuple->value->cstring);
    set_error(iss_data, error_tuple->value->cstring);
  }
  else if (positions_tuple) {
    SkyPosition *new_positions = load_positions(received);
    APP_LOG(APP_LOG_LEVEL_INFO, "Got %i positions", sp_list_count(new_positions));
    if (iss_data->position_list == NULL) {
      iss_data->position_list = new_positions;
    } else {
      sp_list_append(iss_data->position_list, new_positions);
    }
  }
  else {
    Tuple *first = dict_read_first(received);
    if (first)
      APP_LOG(APP_LOG_LEVEL_WARNING, "Got unknown message with first ID=%li", first->key);
    else
      APP_LOG(APP_LOG_LEVEL_WARNING, "Got empty message (size=%li)", dict_size(received));
  }
}

static SkyPosition *load_positions(DictionaryIterator *received) {
  Tuple *timestamp, *azimuth, *elevation;

  SkyPosition *list_head = NULL;

  bool done = false;
  int i = 0;
  while (!done) {
    timestamp = dict_find(received, KEY_POSITIONS_OFFSET + i * 3 + POSITION_TIME);
    azimuth = dict_find(received, KEY_POSITIONS_OFFSET + i * 3 + POSITION_AZIMUTH);
    elevation = dict_find(received, KEY_POSITIONS_OFFSET + i * 3 + POSITION_ELEVATION);

    if (timestamp && azimuth && elevation) {
      SkyPosition *pos = malloc(sizeof(SkyPosition));
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Got pos i=%i t=%li a=%li e=%li", i, timestamp->value->int32, azimuth->value->int32, elevation->value->int32);
      pos->timestamp = timestamp->value->int32;
      pos->azimuth = azimuth->value->int32;
      pos->elevation = elevation->value->int32;
      pos->next = NULL;

      if (list_head == NULL) {
        list_head = pos;
      }
      else {
        sp_list_append(list_head, pos);
      }
      i++;
    }
    else {
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "Done reading positions at i=%i t=%p a=%p e=%p", i, timestamp, azimuth, elevation);
      done = true;
    }
  }
  return list_head;
}

static void appmsg_in_dropped(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "In dropped: %i - %s", reason, translate_error(reason));
}

static void appmsg_out_sent(DictionaryIterator *sent, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Out sent.");
}

static void appmsg_out_failed(DictionaryIterator *failed, AppMessageResult reason, void *context) {
  ISSData *iss_data = (ISSData *) context;

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Out failed: %i", reason);

  switch (reason) {
    case APP_MSG_SEND_TIMEOUT:
      request_update();
      break;
    case APP_MSG_SEND_REJECTED:
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

  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  request_update();
}

void deinit_comm() {
  app_message_deregister_callbacks();
}
