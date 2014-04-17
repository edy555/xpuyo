#include <X11/Intrinsic.h>
#include "puyo.h"

#ifndef __PUYO_BITMAP_H
#define __PUYO_BITMAP_H

#define PBITMAP_SURPRISING_FRAME	16
#define PBITMAP_BOUNDING_START_FRAME	17
#define PBITMAP_BOUNDING_NUM_FRAMES     3

#define PBITMAP_NUM_FRAMES \
  (PUYO_NUM_CONNECTIONS + 1 + PBITMAP_BOUNDING_NUM_FRAMES)

#define PBITMAP_OBSTACLE_NORMAL_FRAME		0
#define PBITMAP_OBSTACLE_SURPRISING_FRAME	1
#define	PBITMAP_OBSTACLE_BOUNDING_START_FRAME	2

#define PBITMAP_NUM_OBSTACLE_FRAMES   \
  (1 + 1 + PBITMAP_BOUNDING_NUM_FRAMES)

#define PBITMAP_NUM_SPLASHING_FRAMES \
  PUYO_NUM_SPLASHING_ANIMATION


#define PBITMAP_REDBALL		0
#define PBITMAP_WHITEBALL 	1
#define PBITMAP_MINIBALL        2    
#define PBITMAP_OPOOL_BALL_KINDS	3

 /* prototype */

void pbitmap_create_pixmaps (Widget widget);

Pixmap pbitmap_puyo_pixmap (Puyo *p);
GC pbitmap_puyo_gc (Puyo *p);

Pixmap pbitmap_opool_pixmap (int kind);
GC pbitmap_opool_gc (int kind);
int pbitmap_opool_width (int kind);
int pbitmap_opool_height (int kind);


#endif /* __PUYO_BITMAP_H */
