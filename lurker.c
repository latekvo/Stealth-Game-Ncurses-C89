#include "lurker.h"

void update_lurkers(Lurker* lurkers, uint lurker_count, float time_delta) {
  /* In patrol mode, add preference for the lurker to snap to multiples of 45d
  // Add jitter to lurker's rotation, this way it's more realistic and prettier
  // Energy remains constant, when moving slower, it will turns faster.
  // 1. When azimuth target far off from actual, slow down
  // 2. As lurker is slow, it can rotate faster
  // 3. As alignment increases, so does speed
  // NOTE: Using accel would yield more realistic look, i can imagine stiff rot
  // looking weird, unnatural, will fix this with azimuth target jitter
  // Is p-derived rebound needed? No, direction changes quickly either way
  */

  int i;
  for (i = 0; i < lurker_count; i++) {
    Lurker* lurker = &lurkers[i];
    /* TBD */
  }
}

