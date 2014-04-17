#include "puyo.h"
#include "random.h"

#ifndef __PUYO_BOARD_H
#define __PUYO_BOARD_H

typedef struct puyo_board PuyoBoard;

struct puyo_board {
  int sizex, sizey;
  Puyo **board;
  Puyo *ytbl;

  Random rand;
};

#define PBOARD_PUYO(pboard, x, y)	(&(pboard)->board[y + 2][x + 1])

#define PBOARD_SIZE_X(pboard)		((pboard)->sizex)
#define PBOARD_SIZE_Y(pboard)		((pboard)->sizey)

#define PBOARD_BOARD(pboard)		((pboard)->board)
#define PBOARD_YTBL(pboard)		((pboard)->ytbl)


#define PBOARD_RAND(pboard)		(&(pboard)->rand)


/* prototype */

PuyoBoard *pboard_new (int sx, int sy);
PuyoBoard *pboard_init (PuyoBoard *pb, int sx, int sy);
void pboard_free (PuyoBoard *pb);
PuyoBoard *pboard_copy (PuyoBoard *src, PuyoBoard *dst);

void pboard_set_rand_series (PuyoBoard *pb, int series);

void pboard_clear_board (PuyoBoard *pb);
void pboard_release_bottom (PuyoBoard *pb);

int pboard_count_links (PuyoBoard *pb, int chain);
int pboard_surprising (PuyoBoard *pb);
int pboard_start_splashing (PuyoBoard *pb);
/* boolean pboard_splash (PuyoBoard *pb); */
boolean pboard_press (PuyoBoard *pb);
boolean pboard_fall (PuyoBoard *pb);
boolean pboard_start_falling (PuyoBoard *pb);
boolean pboard_step_falling (PuyoBoard *pb);
boolean pboard_start_obstacle_bounding (PuyoBoard *pb);
boolean pboard_step_obstacle_bounding (PuyoBoard *pb);
boolean pboard_step_splashing (PuyoBoard *pb);
int pboard_put_obstacle_puyo (PuyoBoard *pb, int num);
boolean pboard_there_is_no_falling_puyo_in_prestage (PuyoBoard *pb);

int pboard_apex_height (PuyoBoard *pb);

#endif /* __PUYO_BOARD_H */
