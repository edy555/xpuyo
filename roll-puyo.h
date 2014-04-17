#include "common.h"
#include "direction.h"
#include "random.h"

#include "puyo.h"
#include "puyo-board.h"


#ifndef __ROLL_PUYO_H
#define __ROLL_PUYO_H

typedef struct roll_puyo RollPuyo;

struct roll_puyo {
  PuyoBoard *puyo_board;
  Puyo pivot;
  Puyo satellite;
  Direction direction;		/* direction from pivot to satellite */
  int subdirection;
  int x;			/* location of pivot */
  int y;
  Random rand;
  boolean must_update;

  int state;
  int count;
};

#define ROLLP_PUYO_BOARD(rp)		((rp)->puyo_board)

#define ROLLP_PIVOT_PUYO(rp)		(&(rp)->pivot)
#define ROLLP_SATELLITE_PUYO(rp)	(&(rp)->satellite)

/* direction */

#define ROLLP_DIRECTION(rp)		((rp)->direction)
#define ROLLP_SUBDIRECTION(rp)		((rp)->subdirection)


#define ROLLP_NO_ROTATING	0
#define ROLLP_ROTATING_LEFT	1
#define ROLLP_ROTATING_RIGHT	2

#define ROLLP_STATE(rp)		((rp)->state)
#define ROLLP_COUNT(rp)		((rp)->count)
#define ROLLP_ROTATING(rp)	((rp)->state != ROLLP_NO_ROTATING)


/* location */

#define ROLLP_PIVOT_X(rp)	((rp)->x)
#define ROLLP_PIVOT_Y(rp)	((rp)->y)
#define ROLLP_SATELLITE_X(rp)	((rp)->x + DIR_OFFSET_X (ROLLP_DIRECTION (rp)))
#define ROLLP_SATELLITE_Y(rp)	((rp)->y + DIR_OFFSET_Y (ROLLP_DIRECTION (rp)))

 /* misc */

#define	ROLLP_MUST_UPDATE(rp)	((rp)->must_update)
#define ROLLP_RAND(rp)		(&(rp)->rand)

/* prototypes */

RollPuyo *rollp_new (PuyoBoard *pb);
RollPuyo *rollp_init (RollPuyo *rp, PuyoBoard *pb);
void rollp_free (RollPuyo *rp);
RollPuyo *rollp_copy (RollPuyo *src, RollPuyo *dst);

#if 0
void rollp_rotate_left (RollPuyo *rp);
void rollp_rotate_right (RollPuyo *rp);
#endif
void rollp_move_left (RollPuyo *rp);
void rollp_move_right (RollPuyo *rp);

boolean rollp_start_rotating_left (RollPuyo *rp);
boolean rollp_start_rotating_right (RollPuyo *rp);
boolean rollp_step_rotating (RollPuyo *rp);

boolean rollp_start_bounding (RollPuyo *rp);
boolean rollp_step_bounding (RollPuyo *rp);
void rollp_be_normal (RollPuyo *rp);

boolean rollp_can_put_roll_puyo (RollPuyo *rp);
void rollp_put_roll_puyo_on_board (RollPuyo *rp);
boolean rollp_slidedown (RollPuyo *rp);

void rollp_set_spicies_series (RollPuyo *rp, unsigned int seed);
void rollp_make_roll_puyo (RollPuyo *rp, int x, int y, Direction dir);

#endif /* __ROLL_PUYO_H */
