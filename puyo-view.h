#include <X11/Intrinsic.h>
#include "puyo-game.h"
#include "puyo-board.h"
#include "roll-puyo.h"

#ifndef __PUYO_VIEW_H
#define __PUYO_VIEW_H

typedef struct puyo_view PuyoView;

struct puyo_view {
  Widget frame;

  Widget board;
  Widget next;
  Widget obstacle_announce;

  Widget score_item;
  Widget level_item;

  int piece_width;
  int piece_height;
  int board_width;
  int board_height;

  PuyoGame *puyo_game;

  RollPuyo *old_roll_puyo;
  PuyoBoard *old_puyo_board;

  Boolean picture;
};

#define PVIEW_APP_CONTEXT(pv)		((pv)->app_con)
#define PVIEW_FRAME(pv)			((pv)->frame)

#define PVIEW_BOARD(pv)			((pv)->board)
#define PVIEW_NEXT(pv)			((pv)->next)
#define PVIEW_OBSTACLE_ANNOUNCE(pv)	((pv)->obstacle_announce)

#define PVIEW_SCORE_ITEM(pv)		((pv)->score_item)
#define PVIEW_LEVEL_ITEM(pv)		((pv)->level_item)

#define PVIEW_PIECE_WIDTH(pv)		((pv)->piece_width)
#define PVIEW_PIECE_HEIGHT(pv)		((pv)->piece_height)
#define PVIEW_BOARD_WIDTH(pv)		((pv)->board_width)
#define PVIEW_BOARD_HEIGHT(pv)		((pv)->board_height)

#define PVIEW_PUYO_GAME(pv)		((pv)->puyo_game)

#define PVIEW_OLD_ROLL_PUYO(pv)		((pv)->old_roll_puyo)
#define PVIEW_OLD_PUYO_BOARD(pv)	((pv)->old_puyo_board)

#define PVIEW_PERIOD(pv)		((pv)->period)

  
 /* prototypes */

PuyoView *pview_new (Widget frame, PuyoGame *pg);
PuyoView *pview_init (PuyoView *pv, Widget frame, PuyoGame *pg);
void pview_free (PuyoView *pg);

void pview_refresh_widgets (PuyoView *pv, Widget widget);

void pview_clear (PuyoView *pv);
void pview_display (PuyoView *pv);
void pview_update (PuyoView *pv);

void pview_start (PuyoView *pv);
void pview_pause (PuyoView *pv);

#endif /* __PUYO_VIEW_H */
