#include "puyo.h"
#include "puyo-board.h"
#include "direction.h"


PuyoBoard *
pboard_new (int sx, int sy)
{
  PuyoBoard *pb;

  pb = (PuyoBoard *) xmalloc (sizeof (PuyoBoard));
  return pboard_init (pb, sx, sy);
}

PuyoBoard *
pboard_init (PuyoBoard *pb, int sx, int sy)
{
  int y;

  PBOARD_SIZE_X (pb) = sx;
  PBOARD_SIZE_Y (pb) = sy;

  PBOARD_BOARD (pb) = (Puyo **) xmalloc (sizeof (Puyo *) * (sy + 3));
  PBOARD_YTBL (pb) = (Puyo *) xmalloc (sizeof (Puyo) * (sy + 3) * (sx + 2));

  for (y = 0; y < sy + 3; y++)
    {
      PBOARD_BOARD(pb) [y] = PBOARD_YTBL(pb) + (sx + 2) * y;
    }

  pboard_clear_board (pb);

  rand_init (PBOARD_RAND (pb));

  return pb;
}

void
pboard_free (PuyoBoard *pb)
{
  xfree (PBOARD_BOARD (pb));
  xfree (PBOARD_YTBL (pb));
  xfree (pb);
}

PuyoBoard *
pboard_copy (PuyoBoard *src, PuyoBoard *dst)
{
  if (PBOARD_SIZE_X (src) != PBOARD_SIZE_X (dst)
      || PBOARD_SIZE_Y (src) != PBOARD_SIZE_Y (dst))
    fatal ("pboard_copy:different size between src to dst");

  bcopy (PBOARD_YTBL (src), PBOARD_YTBL (dst),
	 sizeof (Puyo) * (PBOARD_SIZE_Y (src) + 3) * (PBOARD_SIZE_X (src) + 2));
  return dst;
}


void
pboard_set_rand_series (PuyoBoard *pb, int series)
{
  rand_srand (PBOARD_RAND (pb), series);
}

void
pboard_clear_board (PuyoBoard *pb)
{
  int x, y;

  /* initialize puyos on board */
  for (y = -2; y < PBOARD_SIZE_Y (pb) + 1; y++)
    for (x = -1; x < PBOARD_SIZE_X (pb) + 1; x++)
      puyo_init (PBOARD_PUYO (pb, x, y));

  /* set wall */
  for (x = 0; x < PBOARD_SIZE_X (pb); x++)
    {
      PUYO_SPICIES (PBOARD_PUYO (pb, x, -1)) = PUYO_NOBODY;
      PUYO_SPICIES (PBOARD_PUYO (pb, x, -2)) = PUYO_NOBODY;
      PUYO_SPICIES (PBOARD_PUYO (pb, x, PBOARD_SIZE_Y (pb))) = PUYO_WALL;
    }
  for (y = -2; y < PBOARD_SIZE_Y (pb) + 1; y++)
    {
      PUYO_SPICIES (PBOARD_PUYO (pb, -1, y)) = PUYO_WALL;
      PUYO_SPICIES (PBOARD_PUYO (pb, PBOARD_SIZE_X (pb), y)) = PUYO_WALL;
    }
}

void
pboard_release_bottom (PuyoBoard *pb)
{
  int x;

  /* set wall */
  for (x = 0; x < PBOARD_SIZE_X (pb); x++)
    PUYO_SPICIES (PBOARD_PUYO (pb, x, PBOARD_SIZE_Y (pb))) = PUYO_NOBODY;
}


 /* This routine count connected puyos. Have two subroutines. One traces */
 /* connection and count recursively. And the other one propagates the number */
 /* of links that we got recursively too. */

static int
pboard_mark_count (PuyoBoard *pb, int x, int y)
{
  int n = 0;
  Puyo *p = PBOARD_PUYO (pb, x, y);
  Direction dir;

  if (PUYO_MARKED (p))
    return 0;

  PUYO_MARKED (p) = TRUE;
  for (dir = 0; dir < DIR_DIRECTIONS; dir++)
    {
      if (PUYO_CONNECTION(p) & DIR_MASK (dir))
	n += pboard_mark_count (pb, x + DIR_OFFSET_X (dir),
				y + DIR_OFFSET_Y (dir));
    }
  
  return n + 1; /* add myself */
}


 /* pboard_propagate_links assumes that connected puyo's mark is set. */
 /* pboard_mark_count that executed before this routine is called set the mark. */

static void
pboard_propagate_links (PuyoBoard *pb, int x, int y, int links)
{
  Puyo *p = PBOARD_PUYO (pb, x, y);
  Direction dir;

  if (! PUYO_MARKED (p))
    /* already propagated */
    return;

  PUYO_MARKED (p) = FALSE;
  PUYO_LINKS (p) = links;

  for (dir = 0; dir < DIR_DIRECTIONS; dir++)
    {
      if (PUYO_CONNECTION(p) & DIR_MASK (dir))
	pboard_propagate_links (pb, x + DIR_OFFSET_X (dir),
				    y + DIR_OFFSET_Y (dir), links);
    }
}


static void
pboard_connect_puyos (PuyoBoard *pb)
{
  int x, y;
  
  for (y = 0; y < PBOARD_SIZE_Y (pb); y++)
    for (x = 0; x < PBOARD_SIZE_X (pb); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);
	
	PUYO_MARKED(p) = FALSE;
	PUYO_LINKS (p) = 0;
	
	if (PUYO_IS_NORMAL (p))
	  {
	    Direction dir;
	    unsigned connect = 0;
	    
	    for (dir = 0; dir < DIR_DIRECTIONS; dir++)
	      {
		if (PUYO_SPICIES (p) ==
		    PUYO_SPICIES (PBOARD_PUYO (pb,
					       x + DIR_OFFSET_X (dir), 
					       y + DIR_OFFSET_Y (dir))))
		  connect |= DIR_MASK (dir);
	      }
	    
	    if (PUYO_CONNECTION (p) != connect)
	      {
		PUYO_CONNECTION (p) = connect;
		PUYO_MUST_UPDATE (p) = TRUE;
	      }
	  }
      }
}

#define PBOARD_SPLASH_MIN_LINKS	4

 /* This is toplevel routine that counts connected puyos. */
 /* Calc score and returns it, if score is zero, no puyos must be splashed. */

int
pboard_count_links (PuyoBoard *pb, int chain)
{
  int x, y;
  int bonus = 0;
  int sametime = 0;
  int npuyos = 0;
  int base;

  /* build connection and clear links and unset marks. */
  pboard_connect_puyos (pb);
  
  for (y = 0; y < PBOARD_SIZE_Y (pb); y++)
    for (x = 0; x < PBOARD_SIZE_X (pb); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);
	int links;
	
	if (PUYO_IS_NORMAL (p) && PUYO_LINKS (p) == 0)
	  { /* normal puyo and have not been counted yet */
	    links = pboard_mark_count (pb, x, y);
	    pboard_propagate_links (pb, x, y, links);
	    
	    if (links >= PBOARD_SPLASH_MIN_LINKS)
	      sametime++;

	    if (links > 4)
	      bonus += links - 3;
	    if (links == 11)
	      bonus += 2;
	  }

	if (PUYO_LINKS (p) >= PBOARD_SPLASH_MIN_LINKS)
	  npuyos++;
      }

  /* calc score */
  if (sametime > 1)
    bonus += 3 * (1 << sametime) / 4;

  base = npuyos * 10;

  if (chain > 1)
    bonus += 1 << (chain + 1);

  if (bonus == 0)
    bonus = 1;

  return base * bonus;
}


 /* Puyo which links is larger than or equal min_links must be splashed. */
 /* Returns num of splashing puyos. */

int
pboard_start_splashing (PuyoBoard *pb)
{
  int count = 0;
  int x, y;
  Direction dir;

  for (y = 0; y < PBOARD_SIZE_Y (pb); y++)
    for (x = 0; x < PBOARD_SIZE_X (pb); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);

	if (PUYO_LINKS (p) >= PBOARD_SPLASH_MIN_LINKS)
	  {
	    puyo_start_splashing (p);

	    /* search obstacle puyo around splashing normal puyo. */
	    for (dir = 0; dir < DIR_DIRECTIONS; dir++)
	      {
		Puyo *p0 = PBOARD_PUYO (pb, x + DIR_OFFSET_X (dir),
					    y + DIR_OFFSET_Y (dir));

		if (PUYO_IS_OBSTACLE (p0))
		  puyo_start_splashing (p0);
	      }

	    count++;
	  }
      }

  return count;
}


int
pboard_surprising (PuyoBoard *pb)
{
  int count = 0;
  int x, y;
  Direction dir;

  for (y = 0; y < PBOARD_SIZE_Y (pb); y++)
    for (x = 0; x < PBOARD_SIZE_X (pb); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);

	if (PUYO_LINKS (p) >= PBOARD_SPLASH_MIN_LINKS)
	  {
	    puyo_surprising (p);

	    /* search obstacle puyo around surprising normal puyo. */
	    for (dir = 0; dir < DIR_DIRECTIONS; dir++)
	      {
		Puyo *p0 = PBOARD_PUYO (pb, x + DIR_OFFSET_X (dir),
					    y + DIR_OFFSET_Y (dir));

		if (PUYO_IS_OBSTACLE (p0))
		  puyo_surprising (p0);
	      }

	    count++;
	  }
      }

  return count;
}


#if 0		/* obsolated */

 /* All puyo falling to bottom of empty space */

boolean
pboard_press (PuyoBoard *pb)
{
  boolean exist = FALSE;
  int x, y;

  for (y = PBOARD_SIZE_Y (pb) - 1; y >= 0; y--)
    for (x = 0; x < PBOARD_SIZE_X (pb); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);

	if (!PUYO_IS_NOBODY (p))
	  { /* There is puyo here. */
	    int y0;

	    /* search the bottom of empty space under this puyo. */
	    for (y0 = y+1; y0 < PBOARD_SIZE_Y (pb); y0++)
	      {
		if (!PUYO_IS_NOBODY (PBOARD_PUYO (pb, x, y0)))
		  break;
	      }
	    /* now y0 may indicate non empty cell */

	    y0--;
	    /* Is there empty space? */
	    if (y0 != y)
	      { /* yes */
		Puyo *p0 = PBOARD_PUYO (pb, x, y0); /* p0 must be nobody. */

		/* move to bottom of empty space. */
		*p0 = *p;
		PUYO_SPICIES (p) = PUYO_NOBODY;

		PUYO_MUST_UPDATE (p) = TRUE;
		PUYO_MUST_UPDATE (p0) = TRUE;

		exist = TRUE;
	      }
	  }
      }
  return exist;
}

 /* All puyo fall to a empty grid */

boolean
pboard_fall (PuyoBoard *pb)
{
  boolean exist = FALSE;
  int x, y;

  for (y = PBOARD_SIZE_Y (pb) - 1; y >= 0; y--)
    for (x = 0; x < PBOARD_SIZE_X (pb); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);

	if (!PUYO_IS_NOBODY (p))
	  { /* There is puyo here. */
	    Puyo *p0 = PBOARD_PUYO (pb, x, y + 1); /* puyo on lower side. */
	    
	    if (PUYO_IS_NOBODY (p0))
	      {
		/* move to empty space. */
		*p0 = *p;
		PUYO_SPICIES (p) = PUYO_NOBODY;
	    
		PUYO_MUST_UPDATE (p) = TRUE;
		PUYO_MUST_UPDATE (p0) = TRUE;

		exist = TRUE;
	      }
	  }
      }

  return exist;
}

#endif		/* obsolated */



boolean
pboard_start_falling (PuyoBoard *pb)
{
  boolean exist = FALSE;
  int x, y;

  for (y = PBOARD_SIZE_Y (pb) - 1; y >= -1; y--)
    for (x = 0; x < PBOARD_SIZE_X (pb); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);
	Puyo *p0 = PBOARD_PUYO (pb, x, y + 1); /* puyo on lower side. */

	if (!PUYO_IS_NOBODY (p))
	  { /* There is puyo here. */
	    boolean bottom_free = PUYO_IS_NOBODY (p0) || PUYO_FALLING (p0);

	    if (bottom_free)
	      {
		puyo_start_falling (p);
		exist = TRUE;
	      }

	    if (PUYO_BOUNDING (p))
	      exist = TRUE;
	  }
      }

  return exist;
}

boolean
pboard_step_falling (PuyoBoard *pb)
{
  boolean exist = FALSE;
  int x, y;

  for (y = PBOARD_SIZE_Y (pb) - 1; y >= -1; y--)
    for (x = 0; x < PBOARD_SIZE_X (pb); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);
	Puyo *p0 = PBOARD_PUYO (pb, x, y + 1); /* puyo on lower side. */

	if (!PUYO_IS_NOBODY (p))
	  {
	    if (PUYO_FALLING (p))
	      {
		if (puyo_step_falling (p, p0))
		  /* continuing */
		  ;
		else
		  puyo_start_bounding (p);

		exist = TRUE;
	      }
	    else if (PUYO_BOUNDING (p))
	      exist |= puyo_step_bounding (p);
	  }
      }

  return exist;
}



boolean
pboard_start_obstacle_bounding (PuyoBoard *pb)
{
  int x, y;
  boolean exist = FALSE;

  for (y = 0; y < PBOARD_SIZE_Y (pb); y++)
    for (x = 0; x < PBOARD_SIZE_X (pb); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);

	if (PUYO_IS_OBSTACLE (p))
	  {
	    puyo_start_bounding (p);
	    exist = TRUE;
	  }
      }

  return exist;
}


boolean
pboard_step_obstacle_bounding (PuyoBoard *pb)
{
  int x, y;
  boolean exist = FALSE;

  for (y = 0; y < PBOARD_SIZE_Y (pb); y++)
    for (x = 0; x < PBOARD_SIZE_X (pb); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);

	if (PUYO_IS_OBSTACLE (p) && PUYO_BOUNDING (p))
	  exist |= puyo_step_bounding (p);
      }

  return exist;
}


boolean
pboard_step_splashing (PuyoBoard *pb)
{
  int x, y;
  boolean exist = FALSE;

  for (y = 0; y < PBOARD_SIZE_Y (pb); y++)
    for (x = 0; x < PBOARD_SIZE_X (pb); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);

	if (PUYO_SPLASHING (p))
	  exist |= puyo_step_splashing (p);
      }

  return exist;
}





#define PBOARD_PRESTAGE_Y	-1

int
pboard_put_obstacle_puyo (PuyoBoard *pb, int max)
{
  /* preliminary implementation */
  Puyo *p;
  int n = 0;
  int x;

  x = 0;
  if (max > PBOARD_SIZE_X (pb))
    max = PBOARD_SIZE_X (pb);

  while (max > 0 && x < PBOARD_SIZE_X (pb))
    {
      int rest_width = PBOARD_SIZE_X (pb) - x - max;

      if (rest_width != 0)
	x += RAND_INT (PBOARD_RAND (pb), rest_width);

      p = PBOARD_PUYO (pb, x++, PBOARD_PRESTAGE_Y);

      if (max > 0 && PUYO_IS_NOBODY (p))
	{
	  puyo_be_obstacle (p);
	  puyo_start_falling (p);
	}

      max--;
      n++;
    }

  return n;
}

boolean
pboard_there_is_no_falling_puyo_in_prestage (PuyoBoard *pb)
{
  /* preliminary implementation */
  int x;

  for (x = 0; x < PBOARD_SIZE_X (pb); x++)
    {
      Puyo *p = PBOARD_PUYO (pb, x, PBOARD_PRESTAGE_Y);

      if (!PUYO_IS_NOBODY (p) && PUYO_FALLING (p))
	return FALSE;
    }

  return TRUE;
}



int
pboard_apex_height (PuyoBoard *pb)
{
  int x, y;

  for (y = 0; y < PBOARD_SIZE_Y (pb); y++)
    for (x = 0; x < PBOARD_SIZE_X (pb); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);

	if (!PUYO_IS_NOBODY (p))
	  /* calc height, and return */
	  return PBOARD_SIZE_Y (pb) - y - 1;
      }

  return 0;
}
