#include "StdAfx.h"

#include <assert.h>

static unsigned long randseed = 1;

void srandom(unsigned long seed) { randseed = seed; }

/*
 * Pseudo-random number generator for randomizing the profiling clock,
 * and whatever else we might use it for.  The result is uniform on
 * [0, 2^31 - 1].
 */
unsigned long random() {
  long x, hi, lo, t;

  /*
   * Compute x[n + 1] = (7^5 * x[n]) mod (2^31 - 1).
   * From "Random number generators: good ones are hard to find",
   * Park and Miller, Communications of the ACM, vol. 31, no. 10,
   * October 1988, p. 1195.
   */
  x = randseed;
  hi = x / 127773;
  lo = x % 127773;
  t = 16807 * lo - 2836 * hi;
  if (t <= 0)
    t += 0x7fffffff;
  randseed = t;
  return (t);
}

float frandom(float flLow, float flHigh) {
  float fl = float(random()) / float(2147483648.0f); // float in [0,1)
  return (fl * (flHigh - flLow)) + flLow;            // float in [low,high)
}

long random_range(long from, long to) {
  assert(from <= to);
  return ((random() % (to - from + 1)) + from);
}
