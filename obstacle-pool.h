#include "common.h"

#ifndef __OBSTACLE_POOL_H
#define __OBSTACLE_POOL_H

typedef struct obstacle_pool ObstaclePool;

struct obstacle_pool {
  int pool;
  int pending;

  boolean must_update;
};

#define OPOOL_POOL(op)		((op)->pool)
#define OPOOL_PENDING(op)	((op)->pending)

#define OPOOL_MUST_UPDATE(op)	((op)->must_update)

 /* prototypes */

ObstaclePool *opool_new (void);
ObstaclePool *opool_init (ObstaclePool *op);
void opool_free (ObstaclePool *op);

void opool_add_obstacle (ObstaclePool *op, int n);
void opool_set_obstacle (ObstaclePool *op, int n);

boolean opool_have_announce (ObstaclePool *op);
boolean opool_have_pending (ObstaclePool *op);

int opool_puyo_in_pool (ObstaclePool *op);
int opool_puyo_in_pending (ObstaclePool *op);

void opool_put_to_pending (ObstaclePool *op);
int opool_get_from_pending (ObstaclePool *op, int max);

#endif /* __OBSTACLE_POOL_H */
