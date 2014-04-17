#include "common.h"
#include "random.h"

#ifndef __PUYO_H
#define __PUYO_H

typedef struct puyo Puyo;

struct puyo {
  int spicies;			/* type of puyo or nobody, wall */

  boolean bounding;
  boolean splashing;
  boolean falling;
  boolean surprising;

  unsigned animation;
  unsigned count;

  boolean must_update;		/* necessarity of displaying */
  
  unsigned connection;		/* bit mask, state of connection */
  int links;			/* # of connected puyos */
  boolean marked;		/* use for sweeping for count link */

  float offset_x;		/* 0 <= offset < 1 */
  float offset_y;		
};


/* type of puyos */

#define PUYO_WALL	-2
#define PUYO_NOBODY	-1
#define PUYO_1		0	/* red */
#define PUYO_2		1	/* blue */
#define PUYO_3		2	/* green */
#define PUYO_4		3	/* yellow */
#define PUYO_5		4	/* purple ? */
#define PUYO_OBSTACLE	5	/* OJAMA-puyo */

#define PUYO_NUM_NORMAL	5
#define PUYO_NUM_SPICIES 6

#define PUYO_IS_NOBODY(p)		((p)->spicies == PUYO_NOBODY)
#define PUYO_IS_WALL(p)			((p)->spicies == PUYO_WALL)
#define PUYO_IS_NORMAL(p)		(0 <= (p)->spicies && (p)->spicies < PUYO_NUM_NORMAL)
#define PUYO_IS_OBSTACLE(p)		((p)->spicies == PUYO_OBSTACLE)

#define PUYO_SPICIES(p)			((p)->spicies)



#define PUYO_ANIMATION(p)		((p)->animation)
#define PUYO_COUNT(p)			((p)->count)


 /* Drops are fated to be splashed. */

#define PUYO_SPLASHING(p)		((p)->splashing)
#define PUYO_NUM_SPLASHING_ANIMATION	4

#define PUYO_SURPRISING(p)		((p)->surprising)


 /* obstacle animate */

#define PUYO_BOUNDING(p)		((p)->bounding)
#define PUYO_NUM_BOUNDING_ANIMATION	3


 /* falling */

#define PUYO_FALLING(p)			((p)->falling)


 /* connection */

#define PUYO_IS_CONNECTED_TO(dir, p)	((p)->connection & DIR_MASK (dir))
#define PUYO_CONNECT_TO(dir, p)		((p)->connection |= DIR_MASK (dir))

#define PUYO_CONNECTION(p)		((p)->connection)

#define PUYO_NUM_CONNECTIONS		(1 << 4)


#define PUYO_LINKS(p)			((p)->links)


 /* misc */

#define PUYO_MUST_UPDATE(p)		((p)->must_update)
#define PUYO_MARKED(p)			((p)->marked)


 /* geometry */

#define PUYO_FINE_OFFSET_X(p)	((p)->offset_x)
#define PUYO_FINE_OFFSET_Y(p)	((p)->offset_y)


/* prototypes */

Puyo *puyo_new (void);
Puyo *puyo_init (Puyo *p);
void puyo_free (Puyo *p);
Puyo *puyo_copy (Puyo *src, Puyo *dst);
Puyo *puyo_move (Puyo *src, Puyo *dst);

void puyo_choice_spicies (Puyo *p, Random *r);
void puyo_be_obstacle (Puyo *p);
void puyo_be_empty (Puyo *p);
void puyo_be_normal (Puyo *p);

void puyo_surprising (Puyo *p);
void puyo_start_splashing (Puyo *p);
boolean puyo_step_splashing (Puyo *p);
void puyo_start_falling (Puyo *p);
boolean puyo_step_falling (Puyo *p, Puyo *lower);
void puyo_start_bounding (Puyo *p);
boolean puyo_step_bounding (Puyo *p);

#endif /* __PUYO_H */
