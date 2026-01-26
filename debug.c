#include "debug.h"

#include <ncurses.h>

#include "colors.h"
#include "player.h"
#include "utils.h"

void print_fps(float time_delta) {
  uint hz = 1 / time_delta;
  move(0, 1);
  attron(COLOR_PAIR(TEXT_COLOR_CODE));
  printw("fps: %d", hz);
}

void print_frame_number() {
  static uint frame_number = 0;
  move(0, 12);
  attron(COLOR_PAIR(TEXT_COLOR_CODE));
  printw("frame: %d", frame_number);
  frame_number = (frame_number + 1) % 9999;
}

void print_player_data(Player* player) {
  move(0, 25);
  attron(COLOR_PAIR(TEXT_COLOR_CODE));
  printw("Player x: %d y: %d", player->position_x, player->position_y);
}
