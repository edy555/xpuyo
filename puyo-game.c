#include "common.h"

#include "puyo-game.h"
#include "puyo-board.h"
#include "puyo.h"
#include "obstacle-pool.h"

#define ROLLP_INIT_X	2
#define ROLLP_INIT_Y	0
#define ROLLP_INIT_DIR	DIR_UPPER


static boolean pgame_start_counting (PuyoGame *pg);
static void pgame_counting (PuyoGame *pg);

static boolean pgame_start_falling (PuyoGame *pg);
static void pgame_falling (PuyoGame *pg);

static boolean pgame_start_obstacle_falling (PuyoGame *pg);
static void pgame_obstacle_falling (PuyoGame *pg);

static boolean pgame_start_lost_falling (PuyoGame *pg);
static void pgame_lost_falling (PuyoGame *pg);

static boolean pgame_start_splashing (PuyoGame *pg);
static void pgame_splashing (PuyoGame *pg);

static void pgame_put_next_or_obstacle (PuyoGame *pg);
static void pgame_put_next (PuyoGame *pg);

static int pgame_interval (PuyoGame *pg);


PuyoGame *
pgame_new (int width, int height)
{
  PuyoGame *pg = (PuyoGame *) xmalloc (sizeof (PuyoGame));
  return pgame_init (pg, width, height);
}

PuyoGame *
pgame_init (PuyoGame *pg, int width, int height)
{
  PGAME_PUYO_BOARD (pg) = pboard_new (width, height);
  PGAME_ROLL_PUYO (pg) = rollp_new (PGAME_PUYO_BOARD (pg));
  PGAME_NEXT_ROLL_PUYO (pg) = rollp_new (PGAME_PUYO_BOARD (pg));
  PGAME_OBSTACLE_POOL (pg) = opool_new ();

  rand_init (PGAME_RAND (pg));

  PGAME_INTERVAL (pg) = 12;
  PGAME_COUNT (pg) = 0;

  PGAME_RUNNING (pg) = FALSE;
  PGAME_STATE (pg) = PGAME_COUNTING;

  PGAME_OBSTACLE_BOUNDING (pg) = FALSE;
  PGAME_NEXT_BOUNDING (pg) = FALSE;
  PGAME_ROTATING_ROLL_PUYO (pg) = FALSE;

  PGAME_SCORE (pg) = 0;
  PGAME_LEVEL (pg) = 0;
  PGAME_STATUS (pg) = PGAME_NORMAL;
  PGAME_STATUS_MUST_UPDATE (pg) = TRUE;
  PGAME_TMP_SCORE (pg) = 0;

  return pg;
}

void
pgame_free (PuyoGame *pg)
{
  xfree (pg);
}




void
pgame_set_spicies_series (PuyoGame *pg, int series)
{
  rollp_set_spicies_series (PGAME_NEXT_ROLL_PUYO (pg), series);
  pboard_set_rand_series (PGAME_PUYO_BOARD (pg), series);
  rand_srand (PGAME_RAND (pg), series);
}

void
pgame_set_level (PuyoGame *pg, int level)
{
  PGAME_LEVEL (pg) = level;
  PGAME_STATUS_MUST_UPDATE (pg) = TRUE;
}


void
pgame_newgame (PuyoGame *pg)
{
  PGAME_SCORE (pg) = 0;
  PGAME_TMP_SCORE (pg) = 0;
  PGAME_OBSTACLE_PUYOS_MOD (pg) = 0;

  rollp_make_roll_puyo (PGAME_NEXT_ROLL_PUYO (pg),
			ROLLP_INIT_X, ROLLP_INIT_Y, ROLLP_INIT_DIR);

  pboard_clear_board (PGAME_PUYO_BOARD (pg));

  PGAME_TOTAL_SPLASHED_PUYOS (pg) = 0;

  PGAME_OBSTACLE_BOUNDING (pg) = FALSE;
  PGAME_NEXT_BOUNDING (pg) = FALSE;
  PGAME_ROTATING_ROLL_PUYO (pg) = FALSE;

  PGAME_MUST_SEND_OBSTACLE_PUYOS (pg) = 0;

  switch (PGAME_LEVEL (pg))
    {
    case 0: /* very easy */
      opool_set_obstacle (PGAME_OBSTACLE_POOL (pg), 0);
      break;
    case 1: /* easy */
      opool_set_obstacle (PGAME_OBSTACLE_POOL (pg), 0);
      break;
    case 2: /* normal */
      opool_set_obstacle (PGAME_OBSTACLE_POOL (pg), 0);
      break;
    case 3: /* hard */
      opool_set_obstacle (PGAME_OBSTACLE_POOL (pg), 18);
      break;
    case 4: /* very hard */
      opool_set_obstacle (PGAME_OBSTACLE_POOL (pg), 30);
      break;
    }

  PGAME_RUNNING (pg) = TRUE;

  pgame_put_next_or_obstacle (pg);
}

void
pgame_start (PuyoGame *pg)
{
  PGAME_RUNNING (pg) = TRUE;
}

void
pgame_pause (PuyoGame *pg)
{
  PGAME_RUNNING (pg) = FALSE;
}

static boolean
pgame_start_counting (PuyoGame *pg)
{
  PGAME_STATE (pg) = PGAME_COUNTING;
  PGAME_SUBSTATE (pg) = PGAME_COUNT_COUNTING;
  PGAME_COUNT (pg) = 0;

  PGAME_INTERVAL (pg) = pgame_interval (pg);

  return TRUE;
}

static void
pgame_count_counting (PuyoGame *pg)
{
  PGAME_COUNT (pg) ++;

  if (PGAME_COUNT (pg) > PGAME_INTERVAL (pg)
      && !PGAME_OBSTACLE_BOUNDING (pg) && !PGAME_NEXT_BOUNDING (pg)
      && !PGAME_ROTATING_ROLL_PUYO (pg))
    {
      PGAME_COUNT (pg) = 0;

      if (rollp_slidedown (PGAME_ROLL_PUYO (pg)))
	/* executed, roll-puyo have slided */
	;
      else
	{ /* couldn't slidedown */
	  PGAME_SUBSTATE (pg) = PGAME_COUNT_CONTACTING;
	  PGAME_COUNT (pg) = 0;
	}
    }
  else
    {
      if (!PGAME_OBSTACLE_BOUNDING (pg)
	  && RAND_ONEIN (PGAME_RAND (pg), 200))
	PGAME_OBSTACLE_BOUNDING (pg) =
	  pboard_start_obstacle_bounding (PGAME_PUYO_BOARD (pg));
      if (!PGAME_NEXT_BOUNDING (pg)
	  && RAND_ONEIN (PGAME_RAND (pg), 200))
	PGAME_NEXT_BOUNDING (pg) =
	  rollp_start_bounding (PGAME_NEXT_ROLL_PUYO (pg));
    }

  if (PGAME_OBSTACLE_BOUNDING (pg))
    PGAME_OBSTACLE_BOUNDING (pg) =
      pboard_step_obstacle_bounding (PGAME_PUYO_BOARD (pg));
  if (PGAME_NEXT_BOUNDING (pg))
    PGAME_NEXT_BOUNDING (pg) =
      rollp_step_bounding (PGAME_NEXT_ROLL_PUYO (pg));
  if (PGAME_ROTATING_ROLL_PUYO (pg))
    PGAME_ROTATING_ROLL_PUYO (pg) =
      rollp_step_rotating (PGAME_ROLL_PUYO (pg));
}


static void
pgame_put_roll_puyo_and_choose_next_state (PuyoGame *pg)
{
  rollp_put_roll_puyo_on_board (PGAME_ROLL_PUYO (pg));
  
  PGAME_CHAIN (pg) = 0;
  PGAME_TMP_SCORE (pg) = 0;
  
  /* choose next state */
  if (pgame_start_falling (pg))
    ;
  else if (pgame_start_splashing (pg))
    ;
  else
    pgame_put_next_or_obstacle (pg);
}


static void
pgame_count_contacting (PuyoGame *pg)
{
  PGAME_COUNT (pg) ++;

  if (PGAME_COUNT (pg) > 10
      && !PGAME_ROTATING_ROLL_PUYO (pg))
    {
      PGAME_COUNT (pg) = 0;

      if (rollp_slidedown (PGAME_ROLL_PUYO (pg)))
	pgame_start_counting (pg);
      else
	{ /* couldn't slidedown */
	  pgame_put_roll_puyo_and_choose_next_state (pg);
	}
    }

  if (PGAME_ROTATING_ROLL_PUYO (pg))
    PGAME_ROTATING_ROLL_PUYO (pg) =
      rollp_step_rotating (PGAME_ROLL_PUYO (pg));
}

static void
pgame_counting (PuyoGame *pg)
{
  switch (PGAME_SUBSTATE (pg))
    {
    case PGAME_COUNT_COUNTING:
      pgame_count_counting (pg);
      break;

    case PGAME_COUNT_CONTACTING:
      pgame_count_contacting (pg);
      break;

    default:
      fatal ("pgame_counting:Unknown substate");
      break;
    }
}




static boolean
pgame_start_falling (PuyoGame *pg)
{
  if (pboard_start_falling (PGAME_PUYO_BOARD (pg)))
    {
      PGAME_STATE (pg) = PGAME_FALLING;
      return TRUE;
    }	
  else
    return FALSE;
}

static void
pgame_falling (PuyoGame *pg)
{
  if (pboard_step_falling (PGAME_PUYO_BOARD (pg)))
    /* continue falling */
    ;
  else
    { /* choose next state */
      if (pgame_start_splashing (pg))
	;
      else
	pgame_put_next_or_obstacle (pg);
    }
}



static boolean
pgame_start_obstacle_falling (PuyoGame *pg)
{
  PGAME_STATE (pg) = PGAME_OBSTACLE_FALLING;

  return TRUE;
}

static void
pgame_obstacle_falling (PuyoGame *pg)
{
  if (opool_have_pending (PGAME_OBSTACLE_POOL (pg))
      && pboard_there_is_no_falling_puyo_in_prestage(PGAME_PUYO_BOARD (pg)))
    {
      ObstaclePool *opool = PGAME_OBSTACLE_POOL (pg);
      int put_puyos;

      put_puyos = pboard_put_obstacle_puyo (PGAME_PUYO_BOARD (pg),
					    opool_puyo_in_pending (opool));

      opool_get_from_pending (opool, put_puyos);
    }

  if (pboard_step_falling (PGAME_PUYO_BOARD (pg)))
    /* continue falling */
    ;
  else
    pgame_put_next (pg);
}


static boolean
pgame_start_lost_falling (PuyoGame *pg)
{
  pboard_release_bottom (PGAME_PUYO_BOARD (pg));
  pboard_start_falling (PGAME_PUYO_BOARD (pg));

  PGAME_STATE (pg) = PGAME_LOST_FALLING;

  return TRUE;
}

static void
pgame_lost_falling (PuyoGame *pg)
{
  if (pboard_step_falling (PGAME_PUYO_BOARD (pg)))
    /* continue falling */
    ;
  else
    {
      PGAME_RUNNING (pg) = FALSE;
      PGAME_STATUS (pg) = PGAME_LOST;
      PGAME_STATUS_MUST_UPDATE (pg) = FALSE;
    }
}




static boolean
pgame_start_splashing (PuyoGame *pg)
{
  int score;

  PGAME_CHAIN (pg) ++;

  score = pboard_count_links (PGAME_PUYO_BOARD (pg), PGAME_CHAIN (pg));
  if (score > 0)
    {
      PGAME_TMP_SCORE (pg) += score;

      PGAME_STATE (pg) = PGAME_SPLASHING;
      PGAME_SUBSTATE (pg) = PGAME_SPLASH_CONNECTING;
      PGAME_COUNT (pg) = 0;

      if (1 < PGAME_CHAIN (pg))
	PGAME_STATUS (pg) = PGAME_FIRE;
      else if (3 < PGAME_CHAIN (pg))
	PGAME_STATUS (pg) = PGAME_ONEMORE;

      return TRUE;
    }
  else
    return FALSE;
}

static void
pgame_splash_connecting (PuyoGame *pg)
{
  PGAME_COUNT (pg) ++;

  if (PGAME_COUNT (pg) > 15)
    {
      PGAME_SUBSTATE (pg) = PGAME_SPLASH_SURPRISING;
      PGAME_COUNT (pg) = 0;

      pboard_surprising (PGAME_PUYO_BOARD (pg));
    }
}

static void
pgame_splash_surprising (PuyoGame *pg)
{
  int puyos;

  PGAME_COUNT (pg) ++;

  if (PGAME_COUNT (pg) > 15)
    {
      PGAME_SUBSTATE (pg) = PGAME_SPLASH_SPLASHING;
      PGAME_COUNT (pg) = 0;

      puyos = pboard_start_splashing (PGAME_PUYO_BOARD (pg));

      PGAME_TOTAL_SPLASHED_PUYOS (pg) += puyos;
    }
}

static void
pgame_splash_splashing (PuyoGame *pg)
{
  PGAME_COUNT (pg) ++;

  if (PGAME_COUNT (pg) > 5)
    {
      PGAME_COUNT (pg) = 0;
      
      if (pboard_step_splashing (PGAME_PUYO_BOARD (pg)))
	/* continue splashing */
	;
      else
	{
	  if (pgame_start_falling (pg))
	    ;
	  else
	    pgame_put_next_or_obstacle (pg);
	}
    }
}

static void
pgame_splashing (PuyoGame *pg)
{
  switch (PGAME_SUBSTATE (pg))
    {
    case PGAME_SPLASH_CONNECTING:
      pgame_splash_connecting (pg);
      break;

    case PGAME_SPLASH_SURPRISING:
      pgame_splash_surprising (pg);
      break;

    case PGAME_SPLASH_SPLASHING:
      pgame_splash_splashing (pg);
      break;

    default:
      fatal ("pgame_splashing:Unknown substate");
      break;
    }
}





void
pgame_add_obstacle_puyos (PuyoGame *pg, int obstacle_puyos)
{
  opool_add_obstacle (PGAME_OBSTACLE_POOL (pg), obstacle_puyos);
}

int
pgame_must_send_obstacle_puyos (PuyoGame *pg)
{
  return PGAME_MUST_SEND_OBSTACLE_PUYOS (pg);
}

void
pgame_have_sent_obstacle_puyos (PuyoGame *pg)
{
  PGAME_MUST_SEND_OBSTACLE_PUYOS (pg) = 0;
}


static void
pgame_choose_status (PuyoGame *pg)
{
  /* pgame_choose_status decide status, normal or dying */

  int status = pboard_apex_height (PGAME_PUYO_BOARD (pg)) > 8
    ? PGAME_DYING : PGAME_NORMAL;

  if (PGAME_STATUS (pg) != status)
    {
      PGAME_STATUS (pg) = status;
      PGAME_STATUS_MUST_UPDATE (pg) = TRUE;
    }
}

static void
pgame_add_score (PuyoGame *pg)
{
  if (PGAME_TMP_SCORE (pg))
    {
      int total = PGAME_TMP_SCORE (pg) + PGAME_OBSTACLE_PUYOS_MOD (pg);
      int obstacle_puyos = total / 70;

      PGAME_OBSTACLE_PUYOS_MOD (pg) = total % 70;

#if 0
      pgame_add_obstacle_puyo (pg, obstacle_puyos);
#else
      PGAME_MUST_SEND_OBSTACLE_PUYOS (pg) += obstacle_puyos;
#endif

      PGAME_SCORE (pg) += PGAME_TMP_SCORE (pg);
      PGAME_TMP_SCORE (pg) = 0;
      PGAME_STATUS_MUST_UPDATE (pg) = TRUE;
    }
}

static void
pgame_put_next_or_obstacle (PuyoGame *pg)
{
  pgame_add_score (pg);
  pgame_choose_status (pg);

  if (opool_have_announce (PGAME_OBSTACLE_POOL (pg)))
    {
      opool_put_to_pending (PGAME_OBSTACLE_POOL (pg));
      pgame_start_obstacle_falling (pg);
    }
  else
    pgame_put_next (pg);
}

static void
pgame_put_next (PuyoGame *pg)
{
  pgame_start_counting (pg);
  
  rollp_copy (PGAME_NEXT_ROLL_PUYO (pg), PGAME_ROLL_PUYO (pg));
  rollp_make_roll_puyo (PGAME_NEXT_ROLL_PUYO (pg),
			ROLLP_INIT_X, ROLLP_INIT_Y, ROLLP_INIT_DIR);
  
  rollp_be_normal (PGAME_ROLL_PUYO (pg));

  if (! rollp_can_put_roll_puyo (PGAME_ROLL_PUYO (pg)))
    pgame_start_lost_falling (pg);
}


boolean
pgame_step (PuyoGame *pg)
{
  switch (PGAME_STATE (pg))
    {
    case PGAME_COUNTING:
      pgame_counting (pg);
      break;
    case PGAME_FALLING:
      pgame_falling (pg);
      break;
    case PGAME_OBSTACLE_FALLING:
      pgame_obstacle_falling (pg);
      break;
    case PGAME_LOST_FALLING:
      pgame_lost_falling (pg);
      break;
    case PGAME_SPLASHING:
      pgame_splashing (pg);
      break;
      
    default:
      fatal ("pgame_step:unknown puyo-game state");
      break;
    }

  return PGAME_RUNNING (pg);
}


static int
pgame_interval (PuyoGame *pg)
{
  /* preliminary implimentation */
  int interval;

  interval = 10 - PGAME_LEVEL (pg) - PGAME_TOTAL_SPLASHED_PUYOS (pg) / 100;

  if (interval < 2)
    interval = 2;

  return interval;
}

void
pgame_add_winnings (PuyoGame *pg)
{
  PGAME_WINNINGS (pg) ++;
  PGAME_STATUS (pg) = PGAME_WON;
  PGAME_STATUS_MUST_UPDATE (pg) = TRUE;
}

void
pgame_clear_winnings (PuyoGame *pg)
{
  PGAME_WINNINGS (pg) = 0;
}



#define PGAME_CAN_CONTROL(pg)	\
  (PGAME_RUNNING (pg) && PGAME_STATE (pg) == PGAME_COUNTING \
   && !PGAME_ROTATING_ROLL_PUYO (pg))

void
pgame_rotate_left (PuyoGame *pg)
{
  if (PGAME_CAN_CONTROL (pg))
    {
      PGAME_ROTATING_ROLL_PUYO (pg)
	= rollp_start_rotating_left (PGAME_ROLL_PUYO (pg));
    }
}

void
pgame_rotate_right (PuyoGame *pg)
{
  if (PGAME_CAN_CONTROL (pg))
    {
      PGAME_ROTATING_ROLL_PUYO (pg)
	= rollp_start_rotating_right (PGAME_ROLL_PUYO (pg));
    }
}

void
pgame_move_left (PuyoGame *pg)
{
  if (PGAME_CAN_CONTROL (pg))
    {
      rollp_move_left (PGAME_ROLL_PUYO (pg));
    }
}

void
pgame_move_right (PuyoGame *pg)
{
  if (PGAME_CAN_CONTROL (pg))
    {
      rollp_move_right (PGAME_ROLL_PUYO (pg));
    }
}

void
pgame_drop (PuyoGame *pg)
{
  if (PGAME_CAN_CONTROL (pg))
    {
      if (!rollp_slidedown (PGAME_ROLL_PUYO (pg)))
	pgame_put_roll_puyo_and_choose_next_state (pg);
    }
}
