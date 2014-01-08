/* Define the keys that will be used in AppMessage communication.
 * They must match what you define in appinfo.json -> appKeys.
 */
#define KEY_RISETIME        420
#define KEY_DURATION        421
#define KEY_TIMEZONE_OFFSET 422
#define KEY_ERROR           423
#define KEY_REQUEST_UPDATE  424

/* Public functions for the comm module */

void init_comm(ISSData *);
void deinit_comm();

void request_update();
