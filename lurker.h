#ifndef LURKER_H
#define LURKER_H

#include "utils.h"

enum LurkerStatus {
  /* Placeholder */
  STANDING = 0,
  /* Azimuth unaligned, changing frequently (~1s?) */
  WALKING_CAVE,
  /* Azimuth aligned to N*45d, changing rarely (~3s?) */
  WALKING_OFFICE,
};

typedef struct {
  float position_x, position_y;
  float min_velocity, max_velocity;
  float detection_cone_halfangle_rad;
  float azimuth_target_rad, azimuth_current_rad;
  uint status;
  uint patrol_direction_timer;
} Lurker;

#endif
