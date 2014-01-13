#include <pebble.h>

#define MAX_ERROR_LEN 100

struct SkyPosition;

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
  /* Linked list of position of the object in the sky */
  struct SkyPosition *position_list;
  /* Got time delta from phone? */
  bool got_time_delta;
  /* Got error? */
  bool got_error;
  char error_msg[MAX_ERROR_LEN];
} ISSData;

typedef struct SkyPosition {
  time_t timestamp;
  int16_t elevation;
  int16_t azimuth;
  // Last one is NULL
  struct SkyPosition *next;
} SkyPosition;

/* Functions to manipulate the SkyPosition linked list */

void sp_list_append(SkyPosition *head, SkyPosition *element);
void sp_list_free(SkyPosition *current);
int sp_list_count(SkyPosition *current);
