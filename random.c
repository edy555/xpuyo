#include "random.h"
#include "common.h"

#define INIT_SEED	5243
#define MULT		85489613
#define MOD		347234913

Random *
rand_new (void)
{
  Random *r = xmalloc (sizeof (Random));

  return rand_init (r);
}

Random *
rand_init (Random *r)
{
  RAND_SEED (r) = INIT_SEED;

  return r;
}

void
rand_free (Random *r)
{
  xfree (r);
}


Random *
rand_default (void)
{
  static Random *rand_closure = 0;

  if (!rand_closure)
    rand_closure = rand_new ();

  return rand_closure;
}


unsigned int
rand_srand (Random *r, unsigned int seed)
{
  unsigned int old_seed = RAND_SEED (r);
  RAND_SEED (r) = seed;

  return old_seed;	
}

unsigned int
rand_random (Random *r)
{
  unsigned int n = RAND_SEED (r);

  RAND_SEED (r) = (MULT * RAND_SEED (r)) % MOD;

  return n;
}
