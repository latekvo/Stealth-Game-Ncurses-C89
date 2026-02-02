#ifndef DEBUG_H
#define DEBUG_H

#include "lurker.h"
#include "player.h"
#include "utils.h"

void print_fps(float time_delta);
void print_frame_number();
void print_player_data(Player* player);
void print_lurker_data(Lurker* lurkers, uint lurker_count);

#endif
