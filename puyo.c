#include "common.h"
#include "random.h"
#include "puyo.h"



Puyo *
puyo_new (void)
{
  Puyo *p = (Puyo *) xmalloc (sizeof (Puyo));

  return puyo_init (p);
}

Puyo *
puyo_init (Puyo *p)
{
  PUYO_SPICIES (p) = PUYO_NOBODY;
  PUYO_CONNECTION (p) = 0;
  PUYO_LINKS (p) = 0;

  PUYO_ANIMATION (p) = 0;

  PUYO_BOUNDING (p) = FALSE;
  PUYO_SURPRISING (p) = FALSE;
  PUYO_SPLASHING (p) = FALSE;
  PUYO_FALLING (p) = FALSE;

  PUYO_MUST_UPDATE (p) = TRUE;
  PUYO_MARKED (p) = FALSE;

  PUYO_FINE_OFFSET_X (p) = 0.;
  PUYO_FINE_OFFSET_Y (p) = 0.;

  return p;
}
  
void
puyo_free (Puyo *p)
{
  xfree (p);
}

Puyo *
puyo_copy (Puyo *src, Puyo *dst)
{
  *dst = *src;
  return dst;
}

Puyo *
puyo_move (Puyo *src, Puyo *dst)
{
  puyo_copy (src, dst);
  puyo_be_empty (src);

  return dst;
}

void
puyo_choice_spicies (Puyo *p, Random *r)
{
  puyo_init (p);

  /* choose puyo one in normal puyos. */
  PUYO_SPICIES (p) = RAND_INT (r, PUYO_NUM_NORMAL);
}



void
puyo_be_obstacle (Puyo *p)
{
  puyo_init (p);
  PUYO_SPICIES (p) = PUYO_OBSTACLE;

  PUYO_MUST_UPDATE (p) = TRUE;
}

void
puyo_be_empty (Puyo *p)
{
  puyo_init (p);
  PUYO_SPICIES (p) = PUYO_NOBODY;

  PUYO_MUST_UPDATE (p) = TRUE;
}

void
puyo_be_normal (Puyo *p)
{
  PUYO_SURPRISING (p) = FALSE;
  PUYO_SPLASHING (p) = FALSE;
  PUYO_FALLING (p) = FALSE;
  PUYO_BOUNDING (p) = FALSE;
}


void
puyo_surprising (Puyo *p)
{
  PUYO_SURPRISING (p) = TRUE;
  PUYO_SPLASHING (p) = FALSE;
  PUYO_FALLING (p) = FALSE;
  PUYO_BOUNDING (p) = FALSE;

  PUYO_MUST_UPDATE (p) = TRUE;
}

void
puyo_start_splashing (Puyo *p)
{
  PUYO_SPLASHING (p) = TRUE;
  PUYO_SURPRISING (p) = FALSE;
  PUYO_FALLING (p) = FALSE;
  PUYO_BOUNDING (p) = FALSE;
  PUYO_ANIMATION (p) = 0;

  PUYO_MUST_UPDATE (p) = TRUE;
}

boolean
puyo_step_splashing (Puyo *p)
{
  if (PUYO_SPLASHING (p))
    {
      PUYO_ANIMATION (p) ++;
      PUYO_MUST_UPDATE (p) = TRUE;

      if (PUYO_ANIMATION (p) < PUYO_NUM_SPLASHING_ANIMATION)
	return TRUE;
      else
	{
	  PUYO_SPLASHING (p) = FALSE;
	  PUYO_ANIMATION (p) = 0;

	  puyo_be_empty (p);
	  return FALSE;
	}
    }

  return FALSE;
}


void
puyo_start_falling (Puyo *p)
{
  PUYO_FALLING (p) = TRUE;
  PUYO_SPLASHING (p) = FALSE;
  PUYO_SURPRISING (p) = FALSE;
  PUYO_BOUNDING (p) = FALSE;
  PUYO_ANIMATION (p) = 0;

  PUYO_MUST_UPDATE (p) = TRUE;
}

 /* preliminary */
boolean
puyo_step_falling (Puyo *p, Puyo *lower)
{
  boolean bottom_free = PUYO_IS_NOBODY (lower) || PUYO_FALLING (lower);

  if (PUYO_FALLING (p))
    {
      if (bottom_free)
	{
#if 1
	  if (PUYO_ANIMATION (p) < 15)
	    PUYO_FINE_OFFSET_Y (p) += .05 * PUYO_ANIMATION (p) + .25;
	  else
	    PUYO_FINE_OFFSET_Y (p) += 1.;
#else
	  PUYO_FINE_OFFSET_Y (p) += .334;
#endif
	  PUYO_ANIMATION (p) ++;
	  PUYO_MUST_UPDATE (p) = TRUE;

	  if (PUYO_FINE_OFFSET_Y (p) >= 1.0)
	    {
	      PUYO_FINE_OFFSET_Y (p) -= 1.0;
	      puyo_move (p, lower);

	      PUYO_MUST_UPDATE (p) = TRUE;
	      PUYO_MUST_UPDATE (lower) = TRUE;
	    }

	  return TRUE;
	}
      else
	{
	  /* contact ground */

	  PUYO_FINE_OFFSET_Y (p) = 0.;

	  PUYO_ANIMATION (p) = 0;
	  PUYO_MUST_UPDATE (p) = TRUE;
	  PUYO_FALLING (p) = FALSE;


	  PUYO_MUST_UPDATE (lower) = TRUE;

	  return FALSE;
	}
    }

  return FALSE;
}




void
puyo_start_bounding (Puyo *p)
{
  PUYO_BOUNDING (p) = TRUE;
  PUYO_SURPRISING (p) = FALSE;
  PUYO_SPLASHING (p) = FALSE;
  PUYO_FALLING (p) = FALSE;

  PUYO_ANIMATION (p) = 0;
  PUYO_COUNT (p) = 0;

  PUYO_MUST_UPDATE (p) = TRUE;
}

boolean
puyo_step_bounding (Puyo *p)
{
  if (PUYO_BOUNDING (p))
    {
      PUYO_COUNT (p) ++;

      if (PUYO_COUNT (p) % 5 == 0)
	{
	  PUYO_ANIMATION (p) ++;
	  PUYO_MUST_UPDATE (p) = TRUE;
	  
	  if (PUYO_ANIMATION (p) < PUYO_NUM_BOUNDING_ANIMATION)
	    return TRUE;
	  else
	    {
	      PUYO_BOUNDING (p) = FALSE;
	      PUYO_ANIMATION (p) = 0;
	      
	      return FALSE;
	    }
	}
      else
	return TRUE;
    }

  return FALSE;
}

