#include "input_handling.h"

#include <ncurses.h>

void handle_input(Arena* arena, int key) {
  uint d_x = 0, d_y = 0;

  /* TODO: Implement velocity, dampening, etc */

  switch (key) {
    case 'w':
    case KEY_UP:
      d_y = -1;
      break;
    case 'a':
    case KEY_LEFT:
      d_x = -1;
      break;
    case 's':
    case KEY_DOWN:
      d_y = 1;
      break;
    case 'd':
    case KEY_RIGHT:
      d_x = 1;
      break;
    default:
      /* Ignore unknown */
      return;
      break;
  }

  uint pos_x = arena->player->position_x + d_x;
  uint pos_y = arena->player->position_y + d_y;
  uint pos_i = pos_x + pos_y * arena->size_x;

  if (arena->data[pos_i] == FLOOR) {
    arena->player->position_x = pos_x;
    arena->player->position_y = pos_y;
  }
}

