#include <pebble.h>

/* Define a structure to hold of our UI elements */
typedef struct {
  Window *window;

  Layer *skymap_layer;
} PassUI;

/* And some functions to manipulate it */
PassUI *init_pass_ui();
void destroy_pass_ui(PassUI*);

void update_pass_ui(PassUI* pass_ui, ISSData *iss_data);
