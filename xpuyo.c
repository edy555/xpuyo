#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>

#include <stdio.h>

#include "puyo-game.h"
#include "puyo-view.h"

#include "message.h"


#define PORTNAME	"xpuyo-initport"

#define MSG_TICK			1
#define MSG_START_LEVEL_SELECTION	2
#define MSG_LEVEL_SELECTED		8

#define MSG_START			3
#define MSG_PAUSE			4
#define MSG_DISCONNECT			5
#define MSG_LOSTGAME			6
#define MSG_LOSTGAME_REPLY		7

#define MSG_MOVELEFT			10
#define MSG_MOVERIGHT			11
#define MSG_ROTATELEFT			12
#define MSG_ROTATERIGHT			13
#define MSG_DROP			14
#define MSG_OBSTACLEPUYOS_FROM1_TO2	15
#define MSG_OBSTACLEPUYOS_TO2		16


static void xpuyo_timertick (XtPointer closure, XtIntervalId *id);
static void xpuyo_start_timer (void);
static void xpuyo_stop_timer (void);

static void xpuyo_xflush (void);

static void xpuyo_newgame (void);
static void xpuyo_first_start (void);
static void xpuyo_pause (void);
static void xpuyo_start (void);
static void xpuyo_lostgame (void);
static void xpuyo_connect (void);
static void xpuyo_disconnect (void);
static void xpuyo_quit (void);


static void pause_proc (Widget w, XEvent *ev, String *p, Cardinal *np);
static void refresh_proc(Widget w, XEvent *ev,String *p, Cardinal *np);

static void moveleft_proc (Widget w, XEvent *ev, String *p, Cardinal *np);
static void moveright_proc (Widget w, XEvent *ev, String *p, Cardinal *np);
static void rotateleft_proc (Widget w, XEvent *ev, String *p, Cardinal*np);
static void rotateright_proc (Widget w, XEvent *ev, String *p, Cardinal *np);
static void drop_start_proc (Widget w, XEvent *ev, String *p, Cardinal *np);
static void drop_finish_proc (Widget w, XEvent *ev, String *p, Cardinal *np);
static void drop_proc (Widget w, XEvent *ev, String *p, Cardinal *np);

static void send_msg (int type);
static void send_msg_with_arg (int type, void *optional, int length);
static void send_tick (void);
static boolean opponent_play (void);


static void host_dialog_widgets_create (Widget toplevel);
static void connection_error_dialog_widgets_create (Widget toplevel);
static void connection_error (void);

static void level_select (int level);
static void level_menu_widgets_create (Widget button);

static void xpuyo_start_level_selection (void);
static void xpuyo_notified_level_selection (void);

static void start_callback (Widget w,XtPointer client,XtPointer call);
static void connect_callback (Widget w, XtPointer client, XtPointer call);
static void quit_callback (Widget w,XtPointer client,XtPointer call);

static void host_dialog_popup (Widget w, XtPointer client, XtPointer call);
static void host_dialog_ok (Widget w, XtPointer client, XtPointer call);



static XtActionsRec actions[] = {
  {"MoveLeft",	moveleft_proc},
  {"MoveRight",	moveright_proc},
  {"RotateLeft",rotateleft_proc},
  {"RotateRight",rotateright_proc},
  {"DropStart",	drop_start_proc},
  {"DropFinish",drop_finish_proc},
  {"Refresh",	refresh_proc},

  {"Pause",	pause_proc}
};

  
static XrmOptionDescRec options[] = {
  {"-single",	"SingleMode",	XrmoptionNoArg,		"TRUE"},
  {"-host",	"hostname", 	XrmoptionSepArg,	NULL },
  {"-picture",	"picture",	XrmoptionNoArg,		"TRUE"},
  {"-nopicture","picture",	XrmoptionNoArg,		"FALSE"},
  {"-mirror",	"mirror",	XrmoptionNoArg,		"FALSE"},
};

struct xpuyo_resource {
  Dimension piece_width;
  Dimension piece_height;
  Dimension board_width;
  Dimension board_height;

  Boolean single_mode;
  String hostname;
  Boolean picture;
  Boolean mirror;

  unsigned period;	/* interval, mili seconds */

} xpuyo_resources;


#define offset(field) XtOffset(struct xpuyo_resource *, field)

static XtResource resources[] = {
  {"pieceWidth",	"PieceWidth",	XtRDimension,	sizeof (Dimension),
     offset (piece_width),	XtRImmediate,	(XtPointer)24},
  {"pieceHeight",	"PieceHeight",	XtRDimension,	sizeof (Dimension),
     offset (piece_height),	XtRImmediate,	(XtPointer)24},
  {"boardWidth",	"BoardWidth",	XtRDimension,	sizeof (Dimension),
     offset (board_width),	XtRImmediate,	(XtPointer)12},
  {"boardHeight",	"BoardHeight",	XtRDimension,	sizeof (Dimension),
     offset (board_height),	XtRImmediate,	(XtPointer)6},

  {"singleMode",	"SingleMode",	XtRBoolean,	sizeof (Boolean),
     offset (single_mode),	XtRImmediate,	(XtPointer)False},
  {"hostname",		"Hostname",	XtRString,	sizeof (String),
     offset (hostname),		XtRString,	NULL},
  {"picture",		"Picture",	XtRBoolean,	sizeof (Boolean),
     offset (picture),		XtRImmediate,	(XtPointer)False},
  {"mirror",		"Mirror",	XtRBoolean,	sizeof (Boolean),
     offset (mirror),		XtRImmediate,	(XtPointer)False},

  {"period",		"Period",	XtRInt,		sizeof (int),
     offset (period),		XtRImmediate,	(XtPointer)30},
};
#undef offset

static char *fallback_resources[] = {
  NULL,
};


char *programname;
XtAppContext app_con;

static PuyoView *pv1, *pv2;
static PuyoGame *pg1, *pg2;

static int player1_level, player2_level;
static int series_of_puyo;

static Widget start_bt, connect_bt, host_bt, quit_bt;

static Widget host_popup;
static Widget conerr_popup;

static boolean dropping = FALSE;

enum xpuyo_status {
  XPUYO_DISCONNECTED,
  XPUYO_READY,
  XPUYO_PAUSING,
  XPUYO_RUNNING,
  XPUYO_LEVEL_SELECTING
};

static int status = XPUYO_DISCONNECTED;


static void
usage (void)
{
  fprintf (stderr,"usage: %s [Toolkit options] [options]\n", programname);
  fprintf (stderr,"\t-host hostname\t\tspecify connecting host as client.\n");
  fprintf (stderr,"\t-picture/-nopicture\twhether display pictures or not.\n");
  fprintf (stderr,"\t-mirror\tset views mirror side.\n");

  exit (0);
}

void
main(int argc, char *argv[])
{
  Widget toplevel;
  Widget top;
  Widget player1, player2;
  Widget buttons;
    
  programname = argv[0];

  rand_srand (rand_default (), time (0));

  toplevel = XtVaAppInitialize (&app_con, "Xpuyo",
				options, XtNumber (options), &argc, argv,
				fallback_resources, NULL);

  if (argc > 1)
    usage ();

  XtAppAddActions (app_con, actions, XtNumber (actions));

  XtGetApplicationResources (toplevel, (caddr_t) &xpuyo_resources,
			     resources, XtNumber (resources),
			     NULL, (Cardinal) 0);

  top = XtVaCreateManagedWidget ("top", formWidgetClass, toplevel, NULL);
  
  player1 = XtVaCreateManagedWidget ("player1", formWidgetClass, top,
				     NULL);
  player2 = XtVaCreateManagedWidget ("player2", formWidgetClass, top,
				     NULL);
  buttons = XtVaCreateManagedWidget ("buttons", boxWidgetClass, top,
				     NULL);
  start_bt = XtVaCreateManagedWidget ("start", commandWidgetClass, buttons,
				      NULL);
  connect_bt = XtVaCreateManagedWidget ("connect", commandWidgetClass,
					buttons, NULL);
  host_bt = XtVaCreateManagedWidget ("host", commandWidgetClass,
				     buttons, NULL);
  quit_bt = XtVaCreateManagedWidget ("quit", commandWidgetClass, buttons,
				     NULL);
  
  connection_error_dialog_widgets_create (toplevel);
  host_dialog_widgets_create (toplevel);
  level_menu_widgets_create (toplevel);
  
  XtAddCallback(start_bt, XtNcallback, start_callback, toplevel);
  XtAddCallback(quit_bt, XtNcallback, quit_callback, toplevel);
  
  XtAddCallback(connect_bt, XtNcallback, connect_callback, toplevel);

  XtAddCallback(host_bt, XtNcallback, host_dialog_popup, toplevel);
  
  XtInstallAllAccelerators (top, top);
  
  pg1 = pgame_new (xpuyo_resources.board_width,
		   xpuyo_resources.board_height);
  pg2 = pgame_new (xpuyo_resources.board_width,
		   xpuyo_resources.board_height);
  
  if (!xpuyo_resources.mirror)
    {
      pv1 = pview_new (player1, pg1);
      pv2 = pview_new (player2, pg2);
    }
  else
    {
      pv1 = pview_new (player2, pg1);
      pv2 = pview_new (player1, pg2);
    }
  
  if (xpuyo_resources.single_mode)
    {
      XtSetSensitive (connect_bt, False);
      XtSetSensitive (host_bt, False);
      
      status = XPUYO_READY;
    }
  else
    {
      XtSetSensitive(start_bt, False);
    }

  player1_level = 0;
  player2_level = 0;
  
  xpuyo_start_timer ();

  XtRealizeWidget(toplevel);
  XtAppMainLoop (app_con);
}



#define OFF 15

static void
popup_dialog (Widget widget, XtGrabKind grab_kind)
{
  int x, y, garbage;
  unsigned int mask;
  Window junk_window;
  int root_width, root_height;
  Dimension width, height;

  if (!XtIsRealized (widget))
    XtRealizeWidget (widget);

  XQueryPointer(XtDisplay (widget), XtWindow (widget),
		&junk_window, &junk_window,
		&x, &y, &garbage, &garbage, &mask);

  XtVaGetValues (widget, XtNwidth, &width, XtNheight, &height, NULL);

  root_height = HeightOfScreen (XtScreen (widget));
  root_width = WidthOfScreen (XtScreen (widget));

  x = x - width / 2;
  y = y - height / 2;

  if (x < OFF)
    x = OFF;
  if (y < OFF)
    y = OFF;
  if (x + width > root_width - OFF)
    x = root_width - OFF - width;
  if (y + height > root_height - OFF)
    y = root_height - OFF - height;

  XtVaSetValues(widget, XtNx, x, XtNy, y, NULL);
  XtPopup(widget, grab_kind);
}

static char hostname_buf[64];
static Widget hosttext;

static void
host_dialog_widgets_create (Widget toplevel)
{
  Widget form;
  Widget label;
  Widget ok_bt, cancel_bt;

  if (!xpuyo_resources.hostname)
    {
      gethostname (hostname_buf, sizeof (hostname_buf));
      xpuyo_resources.hostname = hostname_buf;
    }

  host_popup = XtVaCreatePopupShell ("hostPopup",
				     transientShellWidgetClass,
				     toplevel, NULL); 
  
  form = XtVaCreateManagedWidget ("hostDialog",
				  formWidgetClass,
				  host_popup, NULL);
  label = XtVaCreateManagedWidget ("label",
				   labelWidgetClass,
				   form, NULL);
  hosttext = XtVaCreateManagedWidget ("hostText",
				      asciiTextWidgetClass,
				      form, NULL);
  ok_bt = XtVaCreateManagedWidget ("ok", commandWidgetClass,
					       form, NULL);

  cancel_bt = XtVaCreateManagedWidget ("cancel", commandWidgetClass,
				       form, NULL);

  XtAddCallback (ok_bt, XtNcallback, host_dialog_ok, (XtPointer)True);
  XtAddCallback (cancel_bt, XtNcallback, host_dialog_ok, (XtPointer)False);

#if 0	/* no mean */
  XtInstallAccelerators (hosttext, ok_bt);
#endif
}

static void
host_dialog_popup (Widget w, XtPointer client_data, XtPointer call_data)
{
  XtVaSetValues (hosttext,
		 XtNstring, xpuyo_resources.hostname,
		 NULL);

  popup_dialog (host_popup, XtGrabExclusive);
}

static void
host_dialog_ok (Widget w, XtPointer client_data, XtPointer call_data)
{
  Boolean ok = (Boolean)client_data;

  XtPopdown (host_popup);

  if (ok)
    XtVaGetValues (hosttext, XtNstring, &xpuyo_resources.hostname,
		   NULL);

  XtSetSensitive(host_bt, TRUE);
}


static void
connect_callback (Widget w, XtPointer client_data, XtPointer call_data)
{
  if (status == XPUYO_DISCONNECTED && !msg_is_connected ())
    xpuyo_connect ();
  else if (status == XPUYO_READY)
    xpuyo_disconnect ();
}


static void
conerr_dialog_ok (Widget w, XtPointer client_data, XtPointer call_data)
{
  XtPopdown (conerr_popup);
}

static void
connection_error (void)
{
  xpuyo_disconnect ();
  popup_dialog (conerr_popup, XtGrabExclusive);
}

static void
connection_error_dialog_widgets_create (Widget toplevel)
{
  Widget dialog, ok_bt;

  conerr_popup = XtVaCreatePopupShell ("conerrPopup",
				       transientShellWidgetClass,
				       toplevel, NULL); 
  dialog = XtVaCreateManagedWidget ("conerrDialog", dialogWidgetClass,
				    conerr_popup, NULL);
  ok_bt = XtVaCreateManagedWidget ("conerrDialogOk", commandWidgetClass,
				   dialog, NULL);
  XtAddCallback (ok_bt, XtNcallback, conerr_dialog_ok, NULL);
}



Widget level_select_popup;

static void
level_select (int level)
{
  player1_level = level;
  send_msg_with_arg (MSG_LEVEL_SELECTED, &level, sizeof level);
  xpuyo_notified_level_selection ();
}

static void
level_menu_select (Widget w, XtPointer client_data, XtPointer call_data)
{
  int level = (int) client_data;

  level_select (level);
  XtPopdown (level_select_popup);
}

static void
level_menu_widgets_create (Widget toplevel)
{
  Widget box;
  int i;

  level_select_popup
    = XtVaCreatePopupShell ("levelMenu", transientShellWidgetClass,
			    toplevel, NULL);

  box = XtVaCreateManagedWidget ("levelMenuBox", boxWidgetClass,
				 level_select_popup, NULL);

  for (i = 0; i < 5; i++)
    {
      char name[16];
      Widget item;
      
      sprintf (name, "item%d", i);
      item = XtVaCreateManagedWidget (name, commandWidgetClass,
				      box, NULL);
      XtAddCallback (item, XtNcallback, level_menu_select, (XtPointer) i);
    }
}

static void
start_callback (Widget w, XtPointer client_data, XtPointer call_data)
{
  switch (status)
    {
    case XPUYO_READY:
      xpuyo_newgame ();
      break;
    case XPUYO_PAUSING:
      xpuyo_start ();
      break;
    case XPUYO_RUNNING:
      xpuyo_pause ();
      break;
    }
}

static void
quit_callback (Widget w, XtPointer client_data, XtPointer call_data)
{
  xpuyo_quit ();

  pview_free (pv1);
  pview_free (pv2);

  XtDestroyApplicationContext (app_con);
  exit(0);
}

static void
xpuyo_start_level_selection (void)
{
  player1_level = -1;
  player2_level = -1;

  popup_dialog (level_select_popup, XtGrabExclusive);

  status = XPUYO_LEVEL_SELECTING;
}

static void
xpuyo_notified_level_selection (void)
{
  if (xpuyo_resources.single_mode
      || (player1_level >= 0 && player2_level >= 0))
    xpuyo_first_start ();
}

static void
xpuyo_quit (void)
{
  send_msg (MSG_DISCONNECT);

  xpuyo_disconnect ();
  xpuyo_stop_timer();
}

static void
xpuyo_first_start (void)
{
  if (status == XPUYO_LEVEL_SELECTING)
    {
      XtSetSensitive (start_bt, True);

      if (xpuyo_resources.single_mode)
	{
	  pgame_set_spicies_series (pg1, series_of_puyo);
	  pgame_set_level (pg1, player1_level);
	  pgame_newgame (pg1);
	}
      else
	{
	  pgame_set_spicies_series (pg1, series_of_puyo);
	  pgame_set_spicies_series (pg2, series_of_puyo);
	  pgame_set_level (pg1, player1_level);
	  pgame_set_level (pg2, player2_level);
	  pgame_newgame (pg1);
	  pgame_newgame (pg2);
	}

      status = XPUYO_PAUSING;
      xpuyo_start ();
    }
}

static void
xpuyo_pause (void)
{
  if (status == XPUYO_RUNNING)
    {
      XtVaSetValues (start_bt, XtNlabel, "Start", NULL);

      if (xpuyo_resources.single_mode)
	pview_pause (pv1);
      else
	{
	  pview_pause (pv1);
	  pview_pause (pv2);
	}

      status = XPUYO_PAUSING;
      send_msg (MSG_PAUSE);
    }
}

static void
xpuyo_start (void)
{
  if (status == XPUYO_PAUSING)
    {
      XtVaSetValues (start_bt, XtNlabel, "Pause", NULL);

      if (xpuyo_resources.single_mode)
	pview_start (pv1);
      else
	{
	  pview_start (pv1);
	  pview_start (pv2);
	}

      status = XPUYO_RUNNING;
      send_msg (MSG_START);
    }
}

static void
xpuyo_newgame (void)
{
  if (status == XPUYO_READY)
    {
      series_of_puyo = rand_random (rand_default ());
      send_msg_with_arg (MSG_START_LEVEL_SELECTION,
			 &series_of_puyo, sizeof series_of_puyo);

      xpuyo_start_level_selection ();
    }
}

static void
xpuyo_lostgame (void)
{
  if (status == XPUYO_RUNNING)
    {
      XtVaSetValues (start_bt, XtNlabel, "Start", NULL);
      status = XPUYO_READY;
    }
}


static void
xpuyo_connect (void)
{
  if (status == XPUYO_DISCONNECTED && !msg_is_connected ())
    {
      if (msg_open_connection (xpuyo_resources.hostname, PORTNAME))
	{
	  XtVaSetValues (connect_bt, XtNlabel, "Disconnect", NULL);

	  /* make new game button sensitive. */
	  XtSetSensitive (start_bt, TRUE);

	  pgame_clear_winnings (pg1);
	  pgame_clear_winnings (pg2);

	  status = XPUYO_READY;
	}
      else
	{
	  connection_error ();
	}
    }
}

static void
xpuyo_disconnect (void)
{
  if (msg_is_connected ())
    {
      msg_close_connection ();
      
      XtVaSetValues (connect_bt, XtNlabel, "Connect", NULL);
      XtVaSetValues (start_bt, XtNlabel, "Start", NULL);
      
      /* make new game button insensitive. */
      XtSetSensitive (start_bt, FALSE);

      status = XPUYO_DISCONNECTED;
    }
}

 /* timers */

static XtIntervalId timer = None;

static void
xpuyo_timertick (XtPointer closure, XtIntervalId *id)
{
  boolean player1_losted = FALSE;
  boolean player2_losted = FALSE;

  xpuyo_start_timer ();

  if (status == XPUYO_RUNNING)
    {
      if (dropping)
	drop_proc (NULL, NULL, NULL, NULL);
  
      if (xpuyo_resources.single_mode)
	{
	  player1_losted = !pgame_step (pg1);
	  if (pgame_must_send_obstacle_puyos (pg1))
	    {
	      pgame_add_obstacle_puyos (pg1,
					pgame_must_send_obstacle_puyos (pg1));
	      pgame_have_sent_obstacle_puyos (pg1);
	    }

	  pview_update (pv1);
	  xpuyo_xflush ();

	  if (player1_losted)
	    xpuyo_lostgame ();
	}
      else
	{
	  send_tick ();
	  player1_losted = !pgame_step (pg1);
	  player2_losted = !opponent_play ();

	  if (pgame_must_send_obstacle_puyos (pg1))
	    {
	      int puyos = pgame_must_send_obstacle_puyos (pg1);

	      send_msg_with_arg (MSG_OBSTACLEPUYOS_FROM1_TO2,
				 &puyos, sizeof puyos);
	      pgame_have_sent_obstacle_puyos (pg1);
	    }

	  if (player1_losted)
	    send_msg (MSG_LOSTGAME);

	  pview_update (pv1);
	  pview_update (pv2);

	  xpuyo_xflush ();

	}
    }
  else if (status != XPUYO_DISCONNECTED)
    {
      opponent_play ();
    }
}

static void
xpuyo_start_timer (void)
{
  timer = XtAppAddTimeOut (app_con, xpuyo_resources.period,
			   xpuyo_timertick, NULL);
}

static void
xpuyo_stop_timer (void)
{
  if (timer)
    {
      XtRemoveTimeOut (timer);
      timer = NULL;
    }
}

static void
xpuyo_xflush (void)
{
  /* preliminary */
  XFlush (XtDisplay (PVIEW_BOARD (pv1)));
}


 /* support funcs */

static void
pause_proc (Widget w, XEvent *event, String *pars, Cardinal *npars)
{
  xpuyo_pause ();
}

static void
refresh_proc (Widget w, XEvent *event, String *pars, Cardinal *npars)
{
  pview_refresh_widgets (pv1, w);
  pview_refresh_widgets (pv2, w);
}

#define XPUYO_CAN_CONTROL()	(status == XPUYO_RUNNING)

static void
moveleft_proc (Widget w, XEvent *event, String *pars, Cardinal *npars)
{
  if (XPUYO_CAN_CONTROL ())
    {
      pgame_move_left (pg1);
      send_msg (MSG_MOVELEFT);
    }
}

static void
moveright_proc (Widget w, XEvent *event, String *pars, Cardinal *npars)
{
  if (XPUYO_CAN_CONTROL ())
    {
      pgame_move_right (pg1);
      send_msg (MSG_MOVERIGHT);
    }
}

static void
rotateleft_proc  (Widget w, XEvent *event, String *pars, Cardinal *npars)
{
  if (XPUYO_CAN_CONTROL ())
    {
      pgame_rotate_left (pg1);
      send_msg (MSG_ROTATELEFT);
    }
}

static void
rotateright_proc  (Widget w, XEvent *event, String *pars, Cardinal *npars)
{
  if (XPUYO_CAN_CONTROL ())
    {
      pgame_rotate_right (pg1);
      send_msg (MSG_ROTATERIGHT);
    }
}

static void
drop_start_proc (Widget w, XEvent *event, String *pars, Cardinal *npars)
{
  dropping = TRUE;
}

static void
drop_finish_proc (Widget w, XEvent *event, String *pars, Cardinal *npars)
{
  dropping = FALSE;
}

static void
drop_proc (Widget w, XEvent *event, String *pars, Cardinal *npars)
{
  if (XPUYO_CAN_CONTROL ())
    {
      pgame_drop (pg1);
      send_msg (MSG_DROP);
    }
}



 /* message passing */

#define BUFFERED_TICK_MAX	15

static int buffered_tick = 0;

static void
send_tick_flush (void)
{
  if (buffered_tick)
    if (msg_send (MSG_TICK, &buffered_tick, sizeof buffered_tick) < 0)
      connection_error ();

  buffered_tick = 0;
}

static void
send_tick (void)
{
  buffered_tick ++;

  if (buffered_tick >= BUFFERED_TICK_MAX)
    send_tick_flush ();
}


static void
send_msg (int type)
{
  send_msg_with_arg (type, NULL, 0);
}

static void
send_msg_with_arg (int type, void *optional, int length)
{
  if (msg_is_connected ())
    {
      send_tick_flush ();
      if (msg_send (type, optional, length) < 0)
	connection_error ();
    }
}

static int pending_tick = 0;
static int tick_in_no_msg = 0;

static boolean
opponent_tick (void)
{
  boolean active = TRUE;

  while (pending_tick > 0 && tick_in_no_msg > 0)
    {
      pending_tick --;
      tick_in_no_msg --;
      active &= pgame_step (pg2);
    }

  if (pending_tick > 0)
    {
      pending_tick --;
      active &= pgame_step (pg2);
    }

  return active;
}

static boolean
opponent_play (void)
{
  char buf[16];
  int type;
  int puyos;

  while (pending_tick == 0 && msg_arrived ())
    {
      type = msg_recv (buf, sizeof buf);

      switch (type)
	{
	case MSG_TICK:
	  pending_tick = *(int *)buf;
	  break;

	case MSG_PAUSE:
	  xpuyo_pause ();
	  break;
	case MSG_START:
	  xpuyo_start ();
	  break;

	case MSG_START_LEVEL_SELECTION:
	  series_of_puyo = *(int *)buf;
	  xpuyo_start_level_selection ();
	  break;
	case MSG_LEVEL_SELECTED:
	  player2_level = *(int *)buf;
	  xpuyo_notified_level_selection ();
	  break;

	case MSG_DISCONNECT:
	  connection_error ();
	  /* xpuyo_disconnect (); */
	  break;

	case MSG_LOSTGAME:
	  xpuyo_lostgame ();
	  /* Opponent lost game */
	  send_msg (MSG_LOSTGAME_REPLY);
	  /* That is I who win */
	  pgame_add_winnings (pg1);
	  break;
	case MSG_LOSTGAME_REPLY:
	  xpuyo_lostgame ();
	  /* opponent got game */
	  pgame_add_winnings (pg2);
	  break;

	case MSG_ROTATELEFT:
	  pgame_rotate_left (pg2);
	  break;
	case MSG_ROTATERIGHT:
	  pgame_rotate_right (pg2);
	  break;
	case MSG_MOVELEFT:
	  pgame_move_left (pg2);
	  break;
	case MSG_MOVERIGHT:
	  pgame_move_right (pg2);
	  break;
	case MSG_DROP:
	  pgame_drop (pg2);
	  break;

	case MSG_OBSTACLEPUYOS_FROM1_TO2:
	  puyos = *(int *)buf;
	  send_msg_with_arg (MSG_OBSTACLEPUYOS_TO2, &puyos, sizeof puyos);
	  pgame_add_obstacle_puyos (pg1, puyos);
	  break;
	case MSG_OBSTACLEPUYOS_TO2:
	  puyos = *(int *)buf;
	  pgame_add_obstacle_puyos (pg2, puyos);
	  break;

	case -1:
	  connection_error ();
	  break;
	default:
	  fatal ("Unknown message recieved from opponent.");
	  break;
	}
    }

  return opponent_tick ();
}    






void
fatal (char *msg)
{
  fprintf (stderr, "%s:%s\n", programname, msg);
  exit (-1);
}
