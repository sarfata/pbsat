#include <pebble.h>

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
  /* Got time delta from phone? */
  bool got_time_delta;
  /* Got error? */
  char *error_msg;
} ISSData;
