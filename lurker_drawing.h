#ifndef LURKER_DRAWING_H
#define LURKER_DRAWING_H

#include "canvas.h"
#include "lurker.h"

void draw_lurkers(Canvas* canvas, Lurker* lurkers, uint lurker_count,
                  float time_delta);

#endif
