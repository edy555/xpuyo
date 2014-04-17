#include "puyo.h"
#include "roll-puyo.h"
#include "obstacle-pool.h"


#ifndef __PUYO_GAME_H
#define __PUYO_GAME_H

typedef struct puyo_game PuyoGame;

struct puyo_game {
  PuyoBoard *puyo_board;
  RollPuyo *roll_puyo;
  RollPuyo *next_roll_puyo;

  ObstaclePool *obstacle_pool;
  Random rand;

  int count;
  int interval;
  int total_splashed_puyos;

  int must_send_obstacle_puyos;

  boolean running;
  boolean obstacle_bounding;
  boolean next_bounding;
  boolean rotating_roll_puyo;

  int state;
  int substate;

  int chain;
  int tmp_score;
  int obstacle_puyos_mod;

  int winnings;
  int score;
  int level;
  int status;

  boolean status_must_update;
};

#define PGAME_PUYO_BOARD(pg)		((pg)->puyo_board)
#define PGAME_ROLL_PUYO(pg)		((pg)->roll_puyo)
#define PGAME_NEXT_ROLL_PUYO(pg)	((pg)->next_roll_puyo)

#define PGAME_OBSTACLE_POOL(pg)		((pg)->obstacle_pool)
#define PGAME_RAND(pg)			(&(pg)->rand)

#define PGAME_COUNT(pg)			((pg)->count)
#define PGAME_INTERVAL(pg)		((pg)->interval)
#define PGAME_TOTAL_SPLASHED_PUYOS(pg) 	((pg)->total_splashed_puyos)
#define PGAME_MUST_SEND_OBSTACLE_PUYOS(pg) \
  					((pg)->must_send_obstacle_puyos)

#define PGAME_RUNNING(pg)		((pg)->running)
#define PGAME_OBSTACLE_BOUNDING(pg) 	((pg)->obstacle_bounding)
#define PGAME_NEXT_BOUNDING(pg)		((pg)->next_bounding)
#define PGAME_ROTATING_ROLL_PUYO(pg) 	((pg)->rotating_roll_puyo)
#define PGAME_CHAIN(pg)			((pg)->chain)
#define PGAME_TMP_SCORE(pg)		((pg)->tmp_score)
#define PGAME_OBSTACLE_PUYOS_MOD(pg)	((pg)->obstacle_puyos_mod)

#define PGAME_WINNINGS(pg)		((pg)->winnings)
#define PGAME_SCORE(pg)			((pg)->score)
#define PGAME_LEVEL(pg)			((pg)->level)
#define PGAME_STATUS(pg)		((pg)->status)
#define PGAME_STATUS_MUST_UPDATE(pg)	((pg)->status_must_update)

enum pgame_status {
  PGAME_WON,
  PGAME_LOST,
  PGAME_NORMAL,
  PGAME_DYING,
  PGAME_FIRE,
  PGAME_ONEMORE
};


#define PGAME_STATE(pg)			((pg)->state)

enum pgame_state {
  PGAME_COUNTING,
  PGAME_FALLING,
  PGAME_OBSTACLE_FALLING,
  PGAME_LOST_FALLING,
  PGAME_SPLASHING
};


#define PGAME_SUBSTATE(pg)		((pg)->substate)

enum pgame_splash_state {
  PGAME_SPLASH_CONNECTING,
  PGAME_SPLASH_SURPRISING,
  PGAME_SPLASH_SPLASHING
};

enum pgame_count_state {
  PGAME_COUNT_COUNTING,
  PGAME_COUNT_CONTACTING
};


 /* protptypes */

PuyoGame * pgame_new (int width, int height);
PuyoGame * pgame_init (PuyoGame *pgame, int width, int height);
void pgame_free (PuyoGame *pg);

void pgame_set_spicies_series (PuyoGame *pg, int series);
void pgame_set_level (PuyoGame *pg, int level);

void pgame_newgame (PuyoGame *pg);

void pgame_start (PuyoGame *pgame);
void pgame_pause (PuyoGame *pgame);

boolean pgame_step (PuyoGame *pgame);

void pgame_add_obstacle_puyos (PuyoGame *pg, int obstacle_puyos);
int pgame_must_send_obstacle_puyos (PuyoGame *pg);
void pgame_have_sent_obstacle_puyos (PuyoGame *pg);

void pgame_add_winnings (PuyoGame *pg);
void pgame_clear_winnings (PuyoGame *pg);

void pgame_rotate_left (PuyoGame *pg);
void pgame_rotate_right (PuyoGame *pg);
void pgame_move_left (PuyoGame *pg);
void pgame_move_right (PuyoGame *pg);
void pgame_drop (PuyoGame *pg);

#endif /* __PUYO_GAME_H */
