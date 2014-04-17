#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include "common.h"
#include "puyo.h"
#include "puyo-bitmap.h"

	/* This file 'bitmap.h' is generated automatically */
	/* from bitmap.def through cpp and sed. */
#include "bitmap.h"


static boolean bitmap_initialized = FALSE;

static Pixmap puyo_pixmap_tbl[PUYO_NUM_NORMAL][PBITMAP_NUM_FRAMES];
static Pixmap obstacle_pixmap_tbl[PBITMAP_NUM_OBSTACLE_FRAMES];
static Pixmap splash_pixmap_tbl[PUYO_NUM_SPICIES][PBITMAP_NUM_SPLASHING_FRAMES];
static Pixmap opool_balls_pixmap_tbl[PBITMAP_OPOOL_BALL_KINDS];
static Pixmap empty;

static GC puyo_gc;


 /* get all bitmap size and bits */

struct xbm_data {
  unsigned char *data;
  unsigned width;
  unsigned height;
};

#ifdef __STDC__
#define XBM_DATA(name) \
  { name##_bits, name##_width, name##_height},
#else
#define QUOTE(x)x
#define XBM_DATA(name)	\
  { QUOTE(name)_bits, QUOTE(name)_width, QUOTE(name)_height},
#endif

#define NORMAL(body, eye)	XBM_DATA(body)
#define OBSTACLE(body, eye)
#define SPLASH(body, eye)
#define OPOOL(body, eye)
#define TILE(tile)
#define OTHER(xbm)

static struct xbm_data body_mask_data[]= {
#include "bitmap.def"
};

#undef NORMAL
#define NORMAL(body, eye)    	XBM_DATA(eye)

static struct xbm_data eye_mask_data[] = {
#include "bitmap.def"
};

#undef NORMAL
#define NORMAL(body, eye)
#undef OBSTACLE
#define OBSTACLE(body, eye)	XBM_DATA(body)

static struct xbm_data obstacle_body_mask_data[] = {
#include "bitmap.def"
};

#undef OBSTACLE
#define OBSTACLE(body, eye)	XBM_DATA(eye)

static struct xbm_data obstacle_eye_mask_data[] = {
#include "bitmap.def"
};

#undef OBSTACLE
#define OBSTACLE(body, eye)
#undef SPLASH
#define SPLASH(body, eye)	XBM_DATA(body)

static struct xbm_data splash_body_data[] = {
#include "bitmap.def"
};

#undef SPLASH
#define SPLASH(body,eye)	XBM_DATA(eye)

static struct xbm_data splash_eye_data[] = {
#include "bitmap.def"
};

#undef SPLASH
#define SPLASH(body, eye)
#undef OPOOL
#define OPOOL(body, eye)	XBM_DATA(body)

static struct xbm_data opool_body_data[] = {
#include "bitmap.def"
};

#undef OPOOL
#define OPOOL(body, eye)	XBM_DATA(eye)

static struct xbm_data opool_eye_data[] = {
#include "bitmap.def"
};

#undef OPOOL
#define OPOOL(body, eye)
#undef TILE
#define TILE(tile)		XBM_DATA(tile)

static struct xbm_data tile_data[] = {
#include "bitmap.def"
};

#undef NORMAL
#undef OBSTACLE
#undef SPLASH
#undef OPOOL
#undef TILE
#undef OTHER




static boolean
pbitmap_is_color_from_x (Widget widget)
{
  Display *dpy = XtDisplay (widget);
  int scr = DefaultScreen (dpy);

  Visual *v = DefaultVisual (dpy, scr);

  /* pull off hand?? */
  if (v->class < StaticColor)
    return FALSE; /* gray or monochrome */
  else
    return TRUE;
}
 
static boolean
pbitmap_is_color (Widget widget)
{
  static boolean is_color;
  static boolean is_color_have_known = FALSE;

  if (!is_color_have_known)
    {
      is_color = pbitmap_is_color_from_x (widget);
      is_color_have_known = TRUE;
    }

  return is_color;
}

static Pixmap
create_pixmap (Display *dpy, Drawable d, int depth,
	       struct xbm_data *body_xbm, struct xbm_data *eye_xbm,
	       GC body_gc, GC eye_gc, GC mask_gc,
	       boolean tiling, Drawable tile, GC tile_gc)
{
  Drawable body, eye;
  Pixmap pm;

  pm = XCreatePixmap (dpy, d, body_xbm->width, body_xbm->height, depth);
  if (pm == None)
    fatal ("create_pixmap:Cannot create pixmap");

  body = XCreateBitmapFromData (dpy, d, body_xbm->data,
				body_xbm->width, body_xbm->height);
  if (body == None)
    fatal ("create_pixmap:Cannot create bitmap");

  eye = XCreateBitmapFromData (dpy, d,
			       eye_xbm->data, eye_xbm->width, eye_xbm->height);
  if (eye == None)
    fatal ("create_pixmap:Cannot create bitmap");

  XCopyPlane (dpy, body, pm, body_gc, 
	      0, 0, body_xbm->width, body_xbm->height, 0, 0, 1);
  if (tiling)
    XCopyPlane (dpy, tile, pm, tile_gc, 
		0, 0, body_xbm->width, body_xbm->height, 0, 0, 1);

  XCopyPlane (dpy, eye, pm, mask_gc, 
	      0, 0, eye_xbm->width, eye_xbm->height, 0, 0, 1);
  XCopyPlane (dpy, eye, pm, eye_gc, 
	      0, 0, eye_xbm->width, eye_xbm->height, 0, 0, 1);

  XFreePixmap (dpy, body);
  XFreePixmap (dpy, eye);

  return pm;
}  

void
pbitmap_create_pixmaps (Widget widget)
{
  Display *dpy = XtDisplay (widget);
  Drawable d = DefaultRootWindow (dpy);
  int depth = DefaultDepth (dpy, DefaultScreen (dpy));

  XGCValues gcv;
  unsigned mask;

  GC body_gc[PUYO_NUM_SPICIES];
  GC eye_gc;
  GC mask_gc;
  Drawable tile[PUYO_NUM_SPICIES];
  GC tile_gc;

  boolean tiling;
  int sp, con;
  int n;

  if (bitmap_initialized)
    return;

  bitmap_initialized = TRUE;

  tiling = !pbitmap_is_color (widget);

  gcv.foreground = 0;
  gcv.background = -1;
  gcv.function = GXand;
  mask = GCForeground | GCBackground | GCFunction;
  mask_gc = XtGetGC (widget, mask, &gcv);

  gcv.foreground = WhitePixel (dpy, DefaultScreen (dpy));
  gcv.background = 0;
  gcv.function = GXor;
  mask = GCForeground | GCBackground | GCFunction;
  eye_gc = XtGetGC (widget, mask, &gcv);

  if (pbitmap_is_color (widget))
    {
#define offset(field) XtOffset (struct puyo_attr *, field)
      struct puyo_attr {
	Pixel   foreground;
	Pixel   background;
      } puyo_attr[PUYO_NUM_SPICIES];

      static XtResource puyo_resources[] = {
	{"foreground",  "Foreground",   XtRPixel,       sizeof(Pixel),
	   offset(foreground),          XtRString,      XtDefaultForeground},
	{"background",	"Background",	XtRPixel,	sizeof(Pixel),
	   offset(background),	        XtRString,	XtDefaultBackground},
      };
#undef offset
      static char *names[PUYO_NUM_SPICIES] = {
	"puyo1", "puyo2", "puyo3", "puyo4", "puyo5", "obstaclePuyo"
	};
      
      for (sp = 0; sp < PUYO_NUM_SPICIES; sp++)
	{
	  XtGetSubresources (widget, (caddr_t) &puyo_attr[sp],
			     names[sp], "Puyo",
			     puyo_resources, XtNumber (puyo_resources),
			     NULL, (Cardinal) 0);
	}

      for (sp = 0; sp < PUYO_NUM_SPICIES; sp++)
	{
	  gcv.foreground = puyo_attr[sp].foreground;
	  gcv.background = puyo_attr[sp].background;
	  gcv.function = GXcopy;
	  mask = GCForeground | GCBackground | GCFunction;

	  body_gc[sp] = XtGetGC (widget, mask, &gcv);
	}
    }
  else
    { /* monochrome */
      gcv.foreground = WhitePixel (dpy, DefaultScreen (dpy));
      gcv.background = BlackPixel (dpy, DefaultScreen (dpy));
      gcv.function = GXcopy;
      mask = GCForeground | GCBackground | GCFunction;

      for (sp = 0; sp < PUYO_NUM_SPICIES; sp++)
	body_gc[sp] = XtGetGC (widget, mask, &gcv);
    }

  if (tiling)
    {
      for (sp = 0; sp < PUYO_NUM_SPICIES; sp++)
	tile[sp]
	  = XCreateBitmapFromData (dpy, d, tile_data[sp].data,
				   tile_data[sp].width, tile_data[sp].height);
      if (tile[sp] == None)
	fatal ("pbitmap_create_pixmaps:Cannot create bitmap");

      if (WhitePixel (dpy, DefaultScreen (dpy)) != 0)
	{
	  gcv.foreground = -1;
	  gcv.background = 0;
	  gcv.function = GXandInverted;
	}
      else
	{
	  gcv.foreground = -1;
	  gcv.background = 0;
	  gcv.function = GXorInverted;
	}

      mask = GCForeground | GCBackground | GCFunction;
      tile_gc = XtGetGC (widget, mask, &gcv);
    }

  for (sp = 0; sp < PUYO_NUM_NORMAL; sp++)
    for (con = 0; con < PBITMAP_NUM_FRAMES; con++)
      puyo_pixmap_tbl[sp][con]
	= create_pixmap (dpy, d, depth,
			 &body_mask_data[sp * PBITMAP_NUM_FRAMES + con],
			 &eye_mask_data[sp * PBITMAP_NUM_FRAMES + con],
			 body_gc[sp], eye_gc, mask_gc,
			 tiling, tile[sp], tile_gc);

  for (n = 0; n < PBITMAP_NUM_OBSTACLE_FRAMES; n++)
    obstacle_pixmap_tbl[n]
      = create_pixmap (dpy, d, depth,
		       &obstacle_body_mask_data[n],
		       &obstacle_eye_mask_data[n],
		       body_gc[PUYO_OBSTACLE], eye_gc, mask_gc,
		       tiling, tile[PUYO_OBSTACLE], tile_gc);

  for (sp = 0; sp < PUYO_NUM_SPICIES; sp++)
    for (n = 0; n < PUYO_NUM_SPLASHING_ANIMATION; n++)
      splash_pixmap_tbl[sp][n]
	= create_pixmap (dpy, d, depth,
			 &splash_body_data[n],
			 &splash_eye_data[n],
			 body_gc[sp], eye_gc, mask_gc,
			 tiling, tile[sp], tile_gc);

  opool_balls_pixmap_tbl[PBITMAP_REDBALL]
    = create_pixmap (dpy, d, depth,
		     &opool_body_data[PBITMAP_REDBALL],
		     &opool_eye_data[PBITMAP_REDBALL],
		     body_gc[0], eye_gc, mask_gc,
		     tiling, tile[0], tile_gc);
  opool_balls_pixmap_tbl[PBITMAP_WHITEBALL]
    = create_pixmap (dpy, d, depth,
		     &opool_body_data[PBITMAP_WHITEBALL],
		     &opool_eye_data[PBITMAP_WHITEBALL],
		     body_gc[5], eye_gc, mask_gc,
		     tiling, tile[5], tile_gc);
  opool_balls_pixmap_tbl[PBITMAP_MINIBALL]
    = create_pixmap (dpy, d, depth,
		     &opool_body_data[PBITMAP_MINIBALL],
		     &opool_eye_data[PBITMAP_MINIBALL],
		     body_gc[5], eye_gc, mask_gc,
		     tiling, tile[5], tile_gc);

  empty = XCreatePixmapFromBitmapData (dpy, d,
				       empty_bits, empty_width, empty_height,
				       0, 0, depth);
  if (empty == None)
    fatal ("pbitmap_create_pixmap:Cannot create pixmap");
    
  XtReleaseGC (widget, mask_gc);
  XtReleaseGC (widget, eye_gc);
  for (sp = 0; sp < PUYO_NUM_SPICIES; sp++)
    XtReleaseGC (widget, body_gc[sp]);

  if (tiling)
    {
      for (sp = 0; sp < PUYO_NUM_SPICIES; sp++)
	XFreePixmap (dpy, tile[sp]);

      XtReleaseGC (widget, tile_gc);
    }

  gcv.function = GXcopy;
  mask = GCFunction;
  puyo_gc = XtGetGC (widget, mask, &gcv);
}



Pixmap
pbitmap_puyo_pixmap (Puyo *p)
{
  Drawable pm = None;

  if (PUYO_SPLASHING (p))
    pm = splash_pixmap_tbl[PUYO_SPICIES (p)][PUYO_ANIMATION (p)];
  else if (PUYO_SURPRISING (p))
    {
      if (PUYO_IS_OBSTACLE (p))
	pm = obstacle_pixmap_tbl[PBITMAP_OBSTACLE_SURPRISING_FRAME];
      else
	pm = puyo_pixmap_tbl[PUYO_SPICIES (p)][PBITMAP_SURPRISING_FRAME];
    }
  else if (PUYO_BOUNDING (p))
    {
      if (PUYO_IS_OBSTACLE (p))
	pm = obstacle_pixmap_tbl[PUYO_ANIMATION (p)
				    + PBITMAP_OBSTACLE_BOUNDING_START_FRAME];
      else
	pm = puyo_pixmap_tbl[PUYO_SPICIES (p)][PUYO_ANIMATION (p)
					     + PBITMAP_BOUNDING_START_FRAME];
    }
  else if (PUYO_IS_OBSTACLE (p))
    pm = obstacle_pixmap_tbl[PBITMAP_OBSTACLE_NORMAL_FRAME];
  else
    pm = puyo_pixmap_tbl[PUYO_SPICIES (p)][PUYO_CONNECTION (p)];

  return pm;
}

GC
pbitmap_puyo_gc (Puyo *p)
{
  return puyo_gc;
}


Pixmap
pbitmap_opool_pixmap (int kind)
{
  return opool_balls_pixmap_tbl[kind];
}

GC
pbitmap_opool_gc (int kind)
{
  return puyo_gc;
}

int
pbitmap_opool_width (int kind)
{
  return opool_body_data[kind].width;
}

int
pbitmap_opool_height (int kind)
{
  return opool_body_data[kind].height;
}
