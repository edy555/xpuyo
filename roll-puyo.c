#include "common.h"
#include "direction.h"
#include "puyo.h"
#include "roll-puyo.h"


RollPuyo *
rollp_new (PuyoBoard *pb)
{
  RollPuyo *rp;

  rp = xmalloc (sizeof (RollPuyo));
  return rollp_init (rp, pb);
}

RollPuyo *
rollp_init (RollPuyo *rp, PuyoBoard *pb)
{
  ROLLP_PUYO_BOARD (rp) = pb;

  puyo_init (ROLLP_PIVOT_PUYO (rp));
  puyo_init (ROLLP_SATELLITE_PUYO (rp));

  ROLLP_DIRECTION (rp) = DIR_UPPER;

  rand_init (ROLLP_RAND (rp));

  ROLLP_MUST_UPDATE (rp) = FALSE;
  return rp;
}

void
rollp_free (RollPuyo *rp)
{
  xfree (rp);
}

RollPuyo *
rollp_copy (RollPuyo *src, RollPuyo *dst)
{
  *dst = *src;

  return dst;
}



static boolean
rollp_can_move_and_rotate (RollPuyo *rp, int x, int y, Direction dir)
{
  Puyo *p;

  /* check the place of pivot */
  p = PBOARD_PUYO (ROLLP_PUYO_BOARD (rp), x, y);
  if (! PUYO_IS_NOBODY (p))
    /* do nothing */
    return FALSE;

  /* calc loc of satellite puyo and check */
  p = PBOARD_PUYO (ROLLP_PUYO_BOARD (rp),
		   x + DIR_OFFSET_X (dir),
		   y + DIR_OFFSET_Y (dir));

  if (! PUYO_IS_NOBODY (p))
    /* do nothing */
    return FALSE;

  return TRUE;
}

static boolean
rollp_can_move (RollPuyo *rp, int x, int y)
{
  return rollp_can_move_and_rotate (rp, x, y, ROLLP_DIRECTION (rp));
}

 /* Only move, not rotate. Because rotatings are execed */
 /* in rollp_step_rotating. */

static boolean
rollp_move_and_can_rotate (RollPuyo *rp, int x, int y, Direction dir)
{
  if (rollp_can_move_and_rotate (rp, x, y, dir))
    {
      ROLLP_PIVOT_X (rp) = x;
      ROLLP_PIVOT_Y (rp) = y;

      ROLLP_MUST_UPDATE (rp) = TRUE;
      return TRUE;
    }
  else
    return FALSE;
}

 /* Check whether rp can do rotating to dir. If rp can rotate from */
 /* movement, rp move. */

static boolean
rollp_can_rotate (RollPuyo *rp, Direction dir)
{
  int x, y;
  Puyo *p;

  /* calc new satellite puyo's loc */
  x = ROLLP_PIVOT_X (rp) + DIR_OFFSET_X (dir);
  y = ROLLP_PIVOT_Y (rp) + DIR_OFFSET_Y (dir);

  p = PBOARD_PUYO (ROLLP_PUYO_BOARD (rp), x, y);
  if (!PUYO_IS_NOBODY (p))
    {
      switch (dir)
	{
	case DIR_UPPER:
	  return rollp_move_and_can_rotate (rp, ROLLP_PIVOT_X (rp),
					    ROLLP_PIVOT_Y (rp) + 1, dir);
	case DIR_LOWER:
	  return rollp_move_and_can_rotate (rp, ROLLP_PIVOT_X (rp),
					    ROLLP_PIVOT_Y (rp) - 1, dir);
	case DIR_LEFT:
	  return rollp_move_and_can_rotate (rp, ROLLP_PIVOT_X (rp) + 1,
					    ROLLP_PIVOT_Y (rp), dir);
	case DIR_RIGHT:
	  return rollp_move_and_can_rotate (rp, ROLLP_PIVOT_X (rp) - 1,
					    ROLLP_PIVOT_Y (rp), dir);
	default:
	  /* do nothing */
	  return FALSE;
	}
    }
    
  return TRUE;
}


void
rollp_move_left (RollPuyo *rp)
{
  if (rollp_can_move (rp, ROLLP_PIVOT_X (rp) - 1, ROLLP_PIVOT_Y (rp)))
    {
      ROLLP_PIVOT_X (rp) --;
      ROLLP_MUST_UPDATE (rp) = TRUE;
    }
}

void
rollp_move_right (RollPuyo *rp)
{
  if (rollp_can_move (rp, ROLLP_PIVOT_X (rp) + 1, ROLLP_PIVOT_Y (rp)))
    {
      ROLLP_PIVOT_X (rp) ++;
      ROLLP_MUST_UPDATE (rp) = TRUE;
    }
}

#if 0			/* obsolated */
static boolean
rollp_rotate (RollPuyo *rp, Direction dir)
{
  int x, y;
  Puyo *p;

  /* calc new satellite puyo's loc */
  x = ROLLP_PIVOT_X (rp) + DIR_OFFSET_X (dir);
  y = ROLLP_PIVOT_Y (rp) + DIR_OFFSET_Y (dir);

  p = PBOARD_PUYO (ROLLP_PUYO_BOARD (rp), x, y);
  if (!PUYO_IS_NOBODY (p))
    {
      switch (dir)
	{
	case DIR_UPPER:
	  return rollp_move_and_rotate (rp, ROLLP_PIVOT_X (rp),
					    ROLLP_PIVOT_Y (rp) + 1, dir);
	case DIR_LOWER:
	  return rollp_move_and_rotate (rp, ROLLP_PIVOT_X (rp),
					    ROLLP_PIVOT_Y (rp) - 1, dir);
	case DIR_LEFT:
	  return rollp_move_and_rotate (rp, ROLLP_PIVOT_X (rp) + 1,
					    ROLLP_PIVOT_Y (rp), dir);
	case DIR_RIGHT:
	  return rollp_move_and_rotate (rp, ROLLP_PIVOT_X (rp) - 1,
					    ROLLP_PIVOT_Y (rp), dir);
	default:
	  /* do nothing */
	  return FALSE;
	}
    }

  ROLLP_DIRECTION (rp) = dir;
  return TRUE;
}

void
rollp_rotate_left (RollPuyo *rp)
{
  rollp_rotate (rp, DIR_ROTATE_LEFT (ROLLP_DIRECTION (rp)));
}

void
rollp_rotate_right (RollPuyo *rp)
{
  rollp_rotate (rp, DIR_ROTATE_RIGHT (ROLLP_DIRECTION (rp)));
}
#endif

static boolean
rollp_can_rotate_left (RollPuyo *rp)
{
  return rollp_can_rotate (rp, DIR_ROTATE_LEFT (ROLLP_DIRECTION (rp)));
}

static boolean
rollp_can_rotate_right (RollPuyo *rp)
{
  return rollp_can_rotate (rp, DIR_ROTATE_RIGHT (ROLLP_DIRECTION (rp)));
}


static void
rollp_set_fine_offset_of_satellite (RollPuyo *rp)
{
  PUYO_FINE_OFFSET_X (ROLLP_SATELLITE_PUYO (rp))
    = dir_fine_offset_x (ROLLP_DIRECTION (rp), ROLLP_SUBDIRECTION (rp))
       - DIR_OFFSET_X (ROLLP_DIRECTION (rp))
	 + PUYO_FINE_OFFSET_X (ROLLP_PIVOT_PUYO (rp));
  PUYO_FINE_OFFSET_Y (ROLLP_SATELLITE_PUYO (rp))
    = dir_fine_offset_y (ROLLP_DIRECTION (rp), ROLLP_SUBDIRECTION (rp))
      - DIR_OFFSET_Y (ROLLP_DIRECTION (rp))
	+ PUYO_FINE_OFFSET_Y (ROLLP_PIVOT_PUYO (rp));
}

boolean
rollp_start_rotating_left (RollPuyo *rp)
{
  if (ROLLP_STATE (rp) == ROLLP_NO_ROTATING && rollp_can_rotate_left (rp))
    {
      ROLLP_SUBDIRECTION (rp) = 0;
      ROLLP_STATE (rp) = ROLLP_ROTATING_LEFT;
      return TRUE;
    }

  return FALSE;
}

boolean
rollp_start_rotating_right (RollPuyo *rp)
{
  if (ROLLP_STATE (rp) == ROLLP_NO_ROTATING && rollp_can_rotate_right (rp))
    {
      ROLLP_SUBDIRECTION (rp) = 0;
      ROLLP_STATE (rp) = ROLLP_ROTATING_RIGHT;
      return TRUE;
    }

  return FALSE;
}

boolean
rollp_step_rotating (RollPuyo *rp)
{
  switch (ROLLP_STATE (rp))
    {
    case ROLLP_ROTATING_LEFT:
      ROLLP_SUBDIRECTION (rp) --;
      break;
    case ROLLP_ROTATING_RIGHT:
      ROLLP_SUBDIRECTION (rp) ++;
      break;
    default:
      return FALSE;
    }

  {
    int x = ROLLP_SATELLITE_X (rp);
    int y = ROLLP_SATELLITE_Y (rp);
    Puyo *p;

    switch (ROLLP_DIRECTION (rp))
      {
      case DIR_UPPER:
	if (ROLLP_STATE (rp) == ROLLP_ROTATING_LEFT)
	  x--;
	else /* right */
	  x++;
	break;
      case DIR_RIGHT:
	if (ROLLP_STATE (rp) == ROLLP_ROTATING_LEFT)
	  y--;
	else /* right */
	  y++;
	break;
      case DIR_LOWER:
	if (ROLLP_STATE (rp) == ROLLP_ROTATING_LEFT)
	  x++;
	else /* right */
	  x--;
	break;
      case DIR_LEFT:
	if (ROLLP_STATE (rp) == ROLLP_ROTATING_LEFT)
	  y++;
	else /* right */
	  y--;
	break;
      }

    p = PBOARD_PUYO (ROLLP_PUYO_BOARD (rp), x, y);
    if (!PUYO_IS_NOBODY (p))
      PUYO_MUST_UPDATE (p) = TRUE;
  }

  ROLLP_MUST_UPDATE (rp) = TRUE;

  if (ROLLP_SUBDIRECTION (rp) == DIR_DIVISION)
    {
      ROLLP_STATE (rp) = ROLLP_NO_ROTATING;
      ROLLP_DIRECTION (rp) = DIR_ROTATE_RIGHT (ROLLP_DIRECTION (rp));
      ROLLP_SUBDIRECTION (rp) = 0;

      rollp_set_fine_offset_of_satellite (rp);
      return FALSE;
    }
  else if (ROLLP_SUBDIRECTION (rp) == - DIR_DIVISION)
    {
      ROLLP_STATE (rp) = ROLLP_NO_ROTATING;
      ROLLP_DIRECTION (rp) = DIR_ROTATE_LEFT (ROLLP_DIRECTION (rp));
      ROLLP_SUBDIRECTION (rp) = 0;

      rollp_set_fine_offset_of_satellite (rp);
      return FALSE;
    }  
  else
    {
      rollp_set_fine_offset_of_satellite (rp);
      return TRUE;
    }
}


boolean
rollp_start_bounding (RollPuyo *rp)
{
  puyo_start_bounding (ROLLP_PIVOT_PUYO (rp));
  puyo_start_bounding (ROLLP_SATELLITE_PUYO (rp));

  ROLLP_MUST_UPDATE (rp) = TRUE;
  return TRUE;
}

boolean
rollp_step_bounding (RollPuyo *rp)
{
  ROLLP_MUST_UPDATE (rp) = TRUE;

  if (puyo_step_bounding (ROLLP_PIVOT_PUYO (rp))
      | puyo_step_bounding (ROLLP_SATELLITE_PUYO (rp)))
    return TRUE;
  else
    return FALSE;
}

void
rollp_be_normal (RollPuyo *rp)
{
  puyo_be_normal (ROLLP_PIVOT_PUYO (rp));
  puyo_be_normal (ROLLP_SATELLITE_PUYO (rp));
}


boolean
rollp_can_put_roll_puyo (RollPuyo *rp)
{
  Puyo *p;

  /* check the place of pivot */
  p = PBOARD_PUYO (ROLLP_PUYO_BOARD (rp),
		   ROLLP_PIVOT_X (rp), ROLLP_PIVOT_Y (rp));
  if (! PUYO_IS_NOBODY (p))
    /* there is puyo */
    return FALSE;

  /* check the place of satellite */
  p = PBOARD_PUYO (ROLLP_PUYO_BOARD (rp),
		   ROLLP_SATELLITE_X (rp), ROLLP_SATELLITE_Y (rp));

  if (! PUYO_IS_NOBODY (p))
    /* there is puyo */
    return FALSE;
  
  return TRUE;
}

void
rollp_put_roll_puyo_on_board (RollPuyo *rp)
{
  Puyo *p;

  p = PBOARD_PUYO (ROLLP_PUYO_BOARD (rp),
		   ROLLP_PIVOT_X (rp), ROLLP_PIVOT_Y (rp));
  puyo_copy (ROLLP_PIVOT_PUYO (rp), p);
  puyo_start_bounding (p);

  p = PBOARD_PUYO (ROLLP_PUYO_BOARD (rp),
		   ROLLP_SATELLITE_X (rp), ROLLP_SATELLITE_Y (rp));
  puyo_copy (ROLLP_SATELLITE_PUYO (rp), p);
  puyo_start_bounding (p);
}
  

boolean
rollp_slidedown (RollPuyo *rp)
{
  Puyo *pivot = ROLLP_PIVOT_PUYO (rp);

  if (PUYO_FINE_OFFSET_Y (pivot) < 0
      || rollp_can_move (rp, ROLLP_PIVOT_X (rp), ROLLP_PIVOT_Y (rp) + 1))
    {
      PUYO_FINE_OFFSET_Y (pivot) += .5;

      if (PUYO_FINE_OFFSET_Y (pivot) > 0)
	{
	  PUYO_FINE_OFFSET_Y (pivot) += -1;
	  ROLLP_PIVOT_Y (rp) ++;
	}

      rollp_set_fine_offset_of_satellite (rp);
      ROLLP_MUST_UPDATE (rp) = TRUE;

      return TRUE;
    }
  else
    { /* contact ground or other puyo */
      return FALSE;
    }
}

void
rollp_set_spicies_series (RollPuyo *rp, unsigned int seed)
{
  rand_srand (ROLLP_RAND (rp), seed);
}

void
rollp_make_roll_puyo (RollPuyo *rp, int x, int y, Direction dir)
{
  ROLLP_PIVOT_X (rp) = x;
  ROLLP_PIVOT_Y (rp) = y;
  ROLLP_DIRECTION (rp) = dir;

  puyo_choice_spicies (ROLLP_PIVOT_PUYO (rp), ROLLP_RAND (rp));
  puyo_choice_spicies (ROLLP_SATELLITE_PUYO (rp), ROLLP_RAND (rp));

  PUYO_FINE_OFFSET_Y (ROLLP_PIVOT_PUYO (rp)) = -.5;
  rollp_set_fine_offset_of_satellite (rp);

  ROLLP_STATE (rp) = ROLLP_NO_ROTATING;
  ROLLP_MUST_UPDATE (rp) = TRUE;
}
