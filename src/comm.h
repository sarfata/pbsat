/* Define the keys that will be used in AppMessage communication.
 * They must match what you define in appinfo.json -> appKeys.
 */
#define KEY_RISETIME        420
#define KEY_SETTIME         421
#define KEY_TIMEZONE_OFFSET 422
#define KEY_ERROR           423
#define KEY_REQUEST_UPDATE  424
#define KEY_POSITIONS_OFFSET 1000

/* We send all the positions info one after the other,
 * starting at KEY_POSITIONS_OFFSET */
#define POSITION_TIME 0
#define POSITION_AZIMUTH 1
#define POSITION_ELEVATION 2


/* Public functions for the comm module */

void init_comm(ISSData *);
void deinit_comm();

void request_update();
