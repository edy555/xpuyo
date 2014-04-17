#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/Simple.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>

#include "puyo-view.h"
#include "puyo-bitmap.h"
#include "common.h"



static void pview_get_resources (PuyoView *pv);
static void pview_create_widgets (PuyoView *pv);

static void pview_put_puyo (PuyoView *pv, Widget widget,
			    int x, int y, Puyo *p, boolean highlight);
static void pview_clear_puyo (PuyoView *pv, Widget widget, int x, int y);
static void pview_clear_board (PuyoView *pv);
static void pview_put_puyo_on_board (PuyoView *pv, int x, int y, Puyo *p,
				     boolean highlight);
static void pview_clear_puyo_on_board (PuyoView *pv, int x, int y, Puyo *p);
static void pview_put_roll_puyo (PuyoView *pv, RollPuyo *rp);
static void pview_clear_roll_puyo (PuyoView *pv, RollPuyo *rp);

static void pview_clear_next (PuyoView *pv);
static void pview_clear_opool (PuyoView *pv);

static void pview_display_board (PuyoView *pv);
static void pview_update_board (PuyoView *pv);
static void pview_display_next (PuyoView *pv);
static void pview_update_next (PuyoView *pv);
static void pview_display_opool (PuyoView *pv);
static void pview_update_opool (PuyoView *pv);
static void pview_display_status (PuyoView *pv);
static void pview_update_status (PuyoView *pv);


PuyoView *
pview_new (Widget frame, PuyoGame *pg)
{
  PuyoView *pv = (PuyoView *) xmalloc (sizeof (PuyoView));

  return pview_init (pv, frame, pg);
}

PuyoView *
pview_init (PuyoView *pv, Widget frame, PuyoGame *pg)
{
  PVIEW_FRAME (pv) = frame;
  PVIEW_PUYO_GAME (pv) = pg;

  pview_get_resources (pv);

  pbitmap_create_pixmaps (frame);

  pview_create_widgets (pv);


  PVIEW_OLD_PUYO_BOARD (pv) = pboard_new (PVIEW_BOARD_WIDTH (pv),
					  PVIEW_BOARD_HEIGHT (pv));
  PVIEW_OLD_ROLL_PUYO (pv) = rollp_new (PGAME_PUYO_BOARD (pg));

  pboard_copy (PGAME_PUYO_BOARD (pg), PVIEW_OLD_PUYO_BOARD (pv));
  rollp_copy (PGAME_ROLL_PUYO (pg), PVIEW_OLD_ROLL_PUYO (pv));

  return pv;
}

void
pview_free (PuyoView *pg)
{
  xfree (pg);
}


static void
pview_get_resources (PuyoView *pv)
{
#define offset(field)	XtOffset (struct puyo_view *, field)
  static XtResource pview_resources[] = {
    {"pieceWidth",	"PieceWidth",	XtRInt,	sizeof (int),
       offset (piece_width),	XtRImmediate,	(XtPointer)24},
    {"pieceHeight",	"PieceHeight",	XtRInt,	sizeof (int),
       offset (piece_height),	XtRImmediate,	(XtPointer)24},
    {"boardWidth",	"BoardWidth",	XtRInt,	sizeof (int),
       offset (board_width),	XtRImmediate,	(XtPointer)12},
    {"boardHeight",	"BoardHeight",	XtRInt,	sizeof (int),
       offset (board_height),	XtRImmediate,	(XtPointer)6},

    {"picture",		"Picture",	XtRBoolean, sizeof (Boolean),
       offset (picture),	XtRImmediate,	(XtPointer)FALSE},
  };
#undef offset

  XtGetApplicationResources (PVIEW_FRAME (pv), (caddr_t) pv,
			     pview_resources, XtNumber (pview_resources),
			     NULL, (Cardinal) 0);
}


static void
pview_create_widgets (PuyoView *pv)
{
  Widget frame = PVIEW_FRAME (pv);
  Widget status;

  PVIEW_OBSTACLE_ANNOUNCE (pv)
    = XtVaCreateManagedWidget ("announce", simpleWidgetClass, frame, 
       XtNwidth, (XtArgVal)(PVIEW_BOARD_WIDTH (pv) * PVIEW_PIECE_WIDTH (pv)),
       XtNheight, (XtArgVal)(2 * PVIEW_PIECE_HEIGHT (pv)),
       NULL);

  PVIEW_BOARD (pv) 
    = XtVaCreateManagedWidget ("board", coreWidgetClass, frame, 
       XtNwidth, (XtArgVal)(PVIEW_BOARD_WIDTH (pv) * PVIEW_PIECE_WIDTH (pv)),
       XtNheight,(XtArgVal)(PVIEW_BOARD_HEIGHT (pv) * PVIEW_PIECE_HEIGHT (pv)),
       NULL);

  PVIEW_NEXT (pv)
    = XtVaCreateManagedWidget ("next", simpleWidgetClass, frame, NULL);

  status = XtVaCreateManagedWidget ("status", formWidgetClass, frame, NULL);

  (void) XtVaCreateManagedWidget ("scLabel", labelWidgetClass, status, NULL);
  PVIEW_SCORE_ITEM (pv) 
    = XtVaCreateManagedWidget ("score", labelWidgetClass, status, NULL);

  (void) XtVaCreateManagedWidget ("lvLabel", labelWidgetClass, status, NULL);
  PVIEW_LEVEL_ITEM (pv)
    = XtVaCreateManagedWidget ("level", labelWidgetClass, status, NULL);
}


 /* drawing puyo */

static void
pview_put_puyo (PuyoView *pv, Widget widget, int x, int y, Puyo *p,
		boolean highlight)
{
  Display *dpy = XtDisplay (widget);
  Window win = XtWindow (widget);

  if (!PUYO_IS_NOBODY (p) && !PUYO_IS_WALL (p))
    {
#if 0				/* not implemented */
 /* pivot puyo highlighted */
      if (highlight)
	XCopyArea (dpy, pbitmap_highlight (), win, pbitmap_highlight_gc (p),
	            0, 0, PVIEW_PIECE_WIDTH (pv), PVIEW_PIECE_HEIGHT (pv),
		    x, y, 1);
#endif
      XCopyArea (dpy, pbitmap_puyo_pixmap (p), win, pbitmap_puyo_gc (p),
		  0, 0, PVIEW_PIECE_WIDTH (pv), PVIEW_PIECE_HEIGHT (pv),
		  x, y);
    }
  else
    XClearArea (dpy, win,
		x, y, PVIEW_PIECE_WIDTH (pv), PVIEW_PIECE_HEIGHT (pv),
		FALSE);
}

static void
pview_clear_puyo (PuyoView *pv, Widget widget, int x, int y)
{
  Display *dpy = XtDisplay (widget);
  Window win = XtWindow (widget);

  XClearArea (dpy, win, x, y, PVIEW_PIECE_WIDTH (pv), PVIEW_PIECE_HEIGHT (pv),
	      FALSE);
}
  
static void
pview_clear_board (PuyoView *pv)
{
  Widget widget = PVIEW_BOARD (pv);
  Display *dpy = XtDisplay (widget);
  Window win = XtWindow (widget);

  /* clear all area */
  XClearArea (dpy, win, 0, 0, 0, 0, FALSE);
}


static void
pview_put_puyo_on_board (PuyoView *pv, int x, int y, Puyo *p,
			 boolean highlight)
{
  int piece_width = PVIEW_PIECE_WIDTH (pv);
  int piece_height = PVIEW_PIECE_HEIGHT (pv);

  pview_put_puyo (pv, PVIEW_BOARD (pv),
		  x * piece_width + PUYO_FINE_OFFSET_X (p) * piece_width,
		  y * piece_height + PUYO_FINE_OFFSET_Y (p) * piece_height,
		  p, highlight);
}

static void
pview_clear_puyo_on_board (PuyoView *pv, int x, int y, Puyo *p)
{
  int piece_width = PVIEW_PIECE_WIDTH (pv);
  int piece_height = PVIEW_PIECE_HEIGHT (pv);

  pview_clear_puyo (pv, PVIEW_BOARD (pv),
		    x * piece_width + PUYO_FINE_OFFSET_X (p) * piece_width,
		    y * piece_height + PUYO_FINE_OFFSET_Y (p) * piece_height);
}
 

static void
pview_put_roll_puyo (PuyoView *pv, RollPuyo *rp)
{
  pview_put_puyo_on_board (pv, ROLLP_PIVOT_X (rp), ROLLP_PIVOT_Y (rp),
			   ROLLP_PIVOT_PUYO (rp), TRUE);
  pview_put_puyo_on_board (pv, ROLLP_SATELLITE_X (rp), ROLLP_SATELLITE_Y (rp),
			   ROLLP_SATELLITE_PUYO (rp), FALSE);
}

static void
pview_clear_roll_puyo (PuyoView *pv, RollPuyo *rp)
{
  pview_clear_puyo_on_board (pv, ROLLP_PIVOT_X (rp), ROLLP_PIVOT_Y (rp),
			     ROLLP_PIVOT_PUYO (rp));
  pview_clear_puyo_on_board (pv, ROLLP_SATELLITE_X (rp),
			     ROLLP_SATELLITE_Y (rp),
			     ROLLP_SATELLITE_PUYO (rp));
}

static void
pview_display_board (PuyoView *pv)
{
  PuyoGame *pg = PVIEW_PUYO_GAME (pv);
  PuyoBoard *pb = PGAME_PUYO_BOARD (pg);
  int x, y;

  pview_clear_board (pv);
  if (!PGAME_RUNNING (pg))
    return;

  for (y = 0; y < PVIEW_BOARD_HEIGHT (pv); y++)
    for (x = 0; x < PVIEW_BOARD_WIDTH (pv); x++)
      {
	Puyo *p = PBOARD_PUYO (pb, x, y);

	pview_put_puyo_on_board (pv, x, y, p, FALSE);

	PUYO_MUST_UPDATE (p) = FALSE;
      }

  {
    RollPuyo *rp = PGAME_ROLL_PUYO (pg);

    pview_put_roll_puyo (pv, rp);
    ROLLP_MUST_UPDATE (rp) = FALSE;
  }

  pboard_copy (PGAME_PUYO_BOARD (pg),
	       PVIEW_OLD_PUYO_BOARD (pv));
  rollp_copy (PGAME_ROLL_PUYO (pg),
	      PVIEW_OLD_ROLL_PUYO (pv));
}

static void
pview_update_board (PuyoView *pv)
{
  int x, y;

  {
    RollPuyo *rp = PGAME_ROLL_PUYO (PVIEW_PUYO_GAME (pv));

    if (ROLLP_MUST_UPDATE (rp))
      {
	RollPuyo *old_rp = PVIEW_OLD_ROLL_PUYO (pv);

	pview_clear_roll_puyo (pv, old_rp);
	pview_put_roll_puyo (pv, rp);

	ROLLP_MUST_UPDATE (rp) = FALSE;
      }
  }

  for (y = 0; y < PVIEW_BOARD_HEIGHT (pv); y++)
    for (x = 0; x < PVIEW_BOARD_WIDTH (pv); x++)
      {
	Puyo *p = PBOARD_PUYO (PGAME_PUYO_BOARD (PVIEW_PUYO_GAME (pv)),
			       x, y);

	if (PUYO_MUST_UPDATE (p))
	  {
	    Puyo *old_p = PBOARD_PUYO (PVIEW_OLD_PUYO_BOARD (pv), x, y);

	    pview_clear_puyo_on_board (pv, x, y, old_p);
	    pview_put_puyo_on_board (pv, x, y, p, FALSE);

	    PUYO_MUST_UPDATE (p) = FALSE;
	  }
      }

  pboard_copy (PGAME_PUYO_BOARD (PVIEW_PUYO_GAME (pv)),
	       PVIEW_OLD_PUYO_BOARD (pv));
  rollp_copy (PGAME_ROLL_PUYO (PVIEW_PUYO_GAME (pv)),
	      PVIEW_OLD_ROLL_PUYO (pv));
}

static void
pview_clear_next (PuyoView *pv)
{
  Widget w = PVIEW_NEXT (pv);

  XClearWindow (XtDisplay (w), XtWindow (w));
}

#define PVIEW_NEXT_X	1
#define PVIEW_NEXT_Y	2

static void
pview_display_next (PuyoView *pv)
{
  PuyoGame *pg = PVIEW_PUYO_GAME (pv);
  RollPuyo *rp = PGAME_NEXT_ROLL_PUYO (pg);
  int piece_width = PVIEW_PIECE_WIDTH (pv);
  int piece_height = PVIEW_PIECE_HEIGHT (pv);

  if (!PGAME_RUNNING (pg))
    {
      pview_clear_next (pv);
      return;
    }

  pview_put_puyo (pv, PVIEW_NEXT (pv),
		  PVIEW_NEXT_X * piece_width, PVIEW_NEXT_Y * piece_height,
		  ROLLP_PIVOT_PUYO (rp), TRUE);
  pview_put_puyo (pv, PVIEW_NEXT (pv),
		  PVIEW_NEXT_X * piece_width, (PVIEW_NEXT_Y-1) * piece_height,
		  ROLLP_SATELLITE_PUYO (rp), FALSE);

  ROLLP_MUST_UPDATE (rp) = FALSE;
}

static void
pview_update_next (PuyoView *pv)
{
  RollPuyo *rp = PGAME_NEXT_ROLL_PUYO (PVIEW_PUYO_GAME (pv));

  if (ROLLP_MUST_UPDATE (rp))
    pview_display_next (pv);
}



static void
pview_clear_opool (PuyoView *pv)
{
  Widget w = PVIEW_OBSTACLE_ANNOUNCE (pv);

  XClearWindow (XtDisplay (w), XtWindow (w));
}

#define REDBALL_PUYOS 	30
#define WHITEBALL_PUYOS 6

static void
pview_display_opool (PuyoView *pv)
{
  PuyoGame *pg = PVIEW_PUYO_GAME (pv);
  ObstaclePool *op = PGAME_OBSTACLE_POOL (pg);
  int puyos = opool_puyo_in_pool (op);
  int x;

  Widget w = PVIEW_OBSTACLE_ANNOUNCE (pv);
  Display *dpy = XtDisplay (w);
  Window win = XtWindow (w);

  pview_clear_opool (pv);

  if (!PGAME_RUNNING (pg))
    return;

  x = 0;
  while (puyos >= REDBALL_PUYOS)
    {
      XCopyArea (dpy, pbitmap_opool_pixmap (PBITMAP_REDBALL), win,
		 pbitmap_opool_gc (PBITMAP_REDBALL),
		 0, 0, pbitmap_opool_width (PBITMAP_REDBALL),
		 pbitmap_opool_height (PBITMAP_REDBALL),
		 x, 0);
      
      x += pbitmap_opool_width (PBITMAP_REDBALL);
      puyos -= REDBALL_PUYOS;
    }
  
  while (puyos >= WHITEBALL_PUYOS)
    {
      XCopyArea (dpy, pbitmap_opool_pixmap (PBITMAP_WHITEBALL), win,
		 pbitmap_opool_gc (PBITMAP_WHITEBALL),
		 0, 0, pbitmap_opool_width (PBITMAP_WHITEBALL),
		 pbitmap_opool_height (PBITMAP_WHITEBALL),
		 x, 0);
      
      x += pbitmap_opool_width (PBITMAP_WHITEBALL);
      puyos -= WHITEBALL_PUYOS;
    }
  
  while (puyos > 0)
    {
      XCopyArea (dpy, pbitmap_opool_pixmap (PBITMAP_MINIBALL), win,
		 pbitmap_opool_gc (PBITMAP_MINIBALL),
		 0, 0, pbitmap_opool_width (PBITMAP_MINIBALL),
		 pbitmap_opool_height (PBITMAP_MINIBALL),
		 x, 0);
      
      x += pbitmap_opool_width (PBITMAP_MINIBALL);
      puyos --;
    }
  
  OPOOL_MUST_UPDATE (op) = FALSE;
}

static void
pview_update_opool (PuyoView *pv)
{
  ObstaclePool *op = PGAME_OBSTACLE_POOL (PVIEW_PUYO_GAME (pv));

  if (OPOOL_MUST_UPDATE (op))
    pview_display_opool (pv);
}

static void
pview_display_status (PuyoView *pv)
{
  static char *level_name[] = {
    "Very Easy", "Easy", "Normal", "Hard", "Vary Hard"
    };
  PuyoGame *pg = PVIEW_PUYO_GAME (pv);
  char buf[256];
  
  sprintf (buf, "%d", PGAME_SCORE (pg));
  XtVaSetValues (PVIEW_SCORE_ITEM (pv), XtNlabel, buf, NULL);
  
  sprintf (buf, "%s", level_name [PGAME_LEVEL (pg)]);
  XtVaSetValues (PVIEW_LEVEL_ITEM (pv), XtNlabel, buf, NULL);
  
  PGAME_STATUS_MUST_UPDATE (pg) = FALSE;
}

static void
pview_update_status (PuyoView *pv)
{
  PuyoGame *pg = PVIEW_PUYO_GAME (pv);

  if (PGAME_STATUS_MUST_UPDATE (pg))
    pview_display_status (pv);
}


void
pview_update (PuyoView *pv)
{
  pview_update_board (pv);
  pview_update_next (pv);
  pview_update_opool (pv);
  pview_update_status (pv);
}

void
pview_display (PuyoView *pv)
{
  pview_display_board (pv);
  pview_display_next (pv);
  pview_display_opool (pv);
}

void
pview_clear (PuyoView *pv)
{
  pview_clear_board (pv);
  pview_clear_next (pv);
  pview_clear_opool (pv);
}


void
pview_refresh_widgets (PuyoView *pv, Widget widget)
{
  if (widget == PVIEW_BOARD (pv))
    pview_display_board (pv);
  else if (widget == PVIEW_NEXT (pv))
    pview_display_next (pv);
  else if (widget == PVIEW_OBSTACLE_ANNOUNCE (pv))
    pview_display_opool (pv);
}


void
pview_start (PuyoView *pv)
{
  pview_display_status (pv);
  pview_display (pv);

  pgame_start (PVIEW_PUYO_GAME (pv));
}

void
pview_pause (PuyoView *pv)
{
  pview_clear (pv);
  pgame_pause (PVIEW_PUYO_GAME (pv));
}
