#include "obstacle-pool.h"
#include "common.h"


/* # of obstacle puyo falling at a time.*/
#define OPOOL_MAX_PENDING	30


ObstaclePool *
opool_new (void)
{
  ObstaclePool *op = (ObstaclePool *) xmalloc (sizeof (ObstaclePool));

  return opool_init (op);
}

ObstaclePool *
opool_init (ObstaclePool *op)
{
  OPOOL_PENDING (op) = 0;
  OPOOL_POOL (op) = 0;

  OPOOL_MUST_UPDATE (op) = FALSE;

  return op;
}

void
opool_free (ObstaclePool *op)
{
  xfree (op);
}


void
opool_add_obstacle (ObstaclePool *op, int n)
{
  OPOOL_POOL (op) += n;
  OPOOL_MUST_UPDATE (op) = TRUE;
}

void
opool_set_obstacle (ObstaclePool *op, int n)
{
  OPOOL_POOL (op) = n;
  OPOOL_MUST_UPDATE (op) = TRUE;
}
 

boolean
opool_have_announce (ObstaclePool *op)
{
  return OPOOL_POOL (op) > 0;
}

boolean
opool_have_pending (ObstaclePool *op)
{
  return OPOOL_PENDING (op) != 0;
}

int
opool_puyo_in_pool (ObstaclePool *op)
{
  return OPOOL_POOL (op);
}

int
opool_puyo_in_pending (ObstaclePool *op)
{
  return OPOOL_PENDING (op);
}

void
opool_put_to_pending (ObstaclePool *op)
{
  if (OPOOL_POOL (op) >= OPOOL_MAX_PENDING)
    {
      OPOOL_POOL (op) -= OPOOL_MAX_PENDING;
      OPOOL_PENDING (op) += OPOOL_MAX_PENDING;
    }
  else
    {
      OPOOL_PENDING (op) += OPOOL_POOL (op);
      OPOOL_POOL (op) = 0;
    }

  OPOOL_MUST_UPDATE (op) = TRUE;
}

int
opool_get_from_pending (ObstaclePool *op, int max)
{
  int result;

  if (OPOOL_PENDING (op) > max)
    {
      OPOOL_PENDING (op) -= max;
      result = max;
    }
  else
    {
      result = OPOOL_PENDING (op);
      OPOOL_PENDING (op) = 0;
    }

  return result;
}
