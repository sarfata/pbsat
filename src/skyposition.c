#include "pbsat.h"

void sp_list_append(SkyPosition *head, struct SkyPosition *element) {
  while (head->next) {
    head = head->next;
  }
  head->next = element;
}

void sp_list_free(SkyPosition *current) {
  while (current) {
    SkyPosition *next = current->next;
    free(current);
    current = next;
  }
}

int sp_list_count(SkyPosition *current) {
  int count = 0;
  while (current) {
    current = current->next;
    count++;
  }
  return count;
}
