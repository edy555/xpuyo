#include "message.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/time.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>

#ifndef errno
extern int errno;
#endif


static int s_id;
static int fs_id;
static struct sockaddr_in sin;
static struct sockaddr_in fsin;

#define ROLE_OF_SERVER	0
#define ROLE_OF_CLIENT	1

static int role;

static int raw_recv (void *msg, int length);
static int raw_send (void *msg, int length);

static void
perror_fatal (label)
     char *label;
{
  perror (label);
  fatal ("");
}

static int
generate_port (portname)
     char *portname;
{
  int port = 0;
  
  while (*portname != '\0')
    port = (port << 1) ^ (*portname++ & 0xFF);
  
  return (port & (0x8000 - 1)) | 0x4000;
}

#ifndef INADDR_NONE
#define INADDR_NONE -1
#endif

static int
setup_addr (char *hostname, char *portname)
{
  long addr;

  bzero (&sin, sizeof sin);
  sin.sin_family = AF_INET;
  sin.sin_port = htons (generate_port (portname));
  
  addr = inet_addr (hostname);
  if (addr != INADDR_NONE)
    sin.sin_addr.s_addr = addr;
  else
    {
      struct hostent *hp;

      hp = gethostbyname (hostname);
      if (hp == NULL)
	return -1;

      bcopy (hp->h_addr, &sin.sin_addr, hp->h_length);
    }

  return 0;
}


boolean
msg_is_connected (void)
{
  return s_id > 0;
}



static int
make_socket (char *hostname, char *portname)
{
  if (setup_addr (hostname, portname) < 0)
    return -1;

  s_id = socket (AF_INET, SOCK_STREAM, 0);
  if (s_id < 0)
    perror_fatal ("msg_open_connection:socket");

  {
    int on = 1;
    if (setsockopt (s_id, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) < 0)
      perror_fatal ("setsockopt");
  }

  if (bind (s_id, &sin, sizeof sin) < 0)
    {
      perror ("bind");

      close (s_id);
      s_id = -1;
      return -1;
    }

  if (listen (s_id, 1) < 0)
    perror_fatal ("msg_open_connection:listen");

  fs_id = accept (s_id, NULL, NULL);
  if (fs_id < 0)
    perror_fatal ("msg_open_connection:accept");

  return 0;
}

static int
connect_socket (char *hostname, char *portname)
{
  int retry;

  if (setup_addr (hostname, portname) < 0)
    return -1;

  for (retry = 10; retry > 0; retry--)
    {
      s_id = socket (AF_INET, SOCK_STREAM, 0);
      if (s_id < 0)
	perror_fatal ("msg_open_connection:socket");

      if (connect (s_id, &sin, sizeof sin) < 0)
	{
	  close (s_id);
	  perror ("connect"), sleep (1);
	}
      else
	break;
    }

  if (retry > 0)
    return 0;
  else
    {
      close (s_id);
      s_id = -1;
      return -1;
    }
}


boolean
msg_open_connection (char *hostname, char *portname)
{
  char buf[64];

  char *username;
  struct passwd *pw_ent;

  char newportname[16];

  pw_ent = getpwuid (getuid ());
  username = pw_ent->pw_name;

  if (!hostname || *hostname == '\0')
    {
      gethostname (buf, sizeof (buf));
      hostname = buf;
    }

  role = ROLE_OF_SERVER;

  if (make_socket (hostname, portname) < 0)
    role = ROLE_OF_CLIENT;

  if (role == ROLE_OF_SERVER)
    {
      char opponent_name[16];

      if (msg_recv (opponent_name, sizeof opponent_name) != MSG_REQUEST)
	{
	  msg_close_connection ();
	  return FALSE;
	}

      sprintf (newportname, "msg%d", time (0) ^ getpid ());

      if (msg_send (MSG_CONNECT, newportname, strlen (newportname) + 1) < 0)
	{
	  msg_close_connection ();
	  return FALSE;
	}

      msg_close_connection ();

      if (make_socket (hostname, newportname) < 0)
	return FALSE;
      else
	return TRUE;
    }
  else
    {
      if (connect_socket (hostname, portname) < 0)
	return FALSE;

      if (msg_send (MSG_REQUEST, username, strlen (username) + 1) < 0)
	{
	  msg_close_connection ();
	  return FALSE;
	}

      if (msg_recv (newportname, sizeof newportname) != MSG_CONNECT)
	{
	  msg_close_connection ();
	  return FALSE;
	}

      msg_close_connection ();

      if (connect_socket (hostname, newportname) < 0)
	return FALSE;
      else
	return TRUE;
    }
}

void
msg_close_connection (void)
{
  if (role == ROLE_OF_SERVER && close (fs_id) < 0)
    perror_fatal ("msg_close_connection:close 2");

  if (close (s_id) < 0)
    perror_fatal ("msg_close_connection:close");

  s_id = -1;
  fs_id = -1;
}

struct msg {
  int type;
  int length;
};

int
msg_send (int msg_type, void *optional, int length)
{
  if (msg_is_connected ())
    {
      struct msg msg;
      int r;
      
      msg.type = msg_type;
      msg.length = length;
      
      r = raw_send (&msg, sizeof msg);
      if (r != sizeof msg)
	return -1;
      
      if (length)
	{
	  r = raw_send (optional, length);
	  if (r != length)
	    return -1;
	}

      return 0;
    }
  else
    return -1;
}
  
int
msg_recv (void *optional, int length)
{
  if (msg_is_connected ())
    {
      struct msg msg;
      int len, rest;
      int r;
      
      r = raw_recv (&msg, sizeof msg);
      if (r != sizeof msg)
	return -1;
      
      if (length < msg.length)
	len = length;
      else
	len = msg.length;
      
      if (optional)
	{
	  r = raw_recv (optional, len);
	  if (r != len)
	    return -1;
	}
      
      rest = msg.length - len;
      while (rest > 0)
	{
	  char dust[128];
	  
	  len = rest < sizeof dust ? rest : sizeof dust;
	  r = raw_recv (dust, len);
	  if (r != len)
	    return -1;
	  
	  rest -= len;
	}
      
      return msg.type;
    }
  else
    return -1;
}
    

static int
check_have_msg (void)
{
  fd_set fds;
  struct timeval tv;
  int nfds;
  int sock = (role == ROLE_OF_SERVER) ? fs_id : s_id;

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  
  FD_ZERO (&fds);
  FD_SET (sock, &fds);
  
  nfds = select (sock + 1, &fds, NULL, NULL, &tv);
  if (nfds < 0)
    perror_fatal ("check_fromserver:select");
  
  return  FD_ISSET (sock, &fds);
}


boolean
msg_arrived (void)
{
  if (msg_is_connected ())
    return check_have_msg ();
  else
    return FALSE;
}



static jmp_buf timeout_env;

static void
timeout (sig)
     int sig;
{
  signal (sig, SIG_IGN);

  signal (SIGALRM, timeout);
  longjmp (timeout_env, 1);
}

static int
raw_recv (void *msg, int len)
{
  int r;
  int sock = (role == ROLE_OF_SERVER) ? fs_id : s_id;

  signal (SIGALRM, timeout);

  if (setjmp (timeout_env) == 0)
    {
      alarm (5);
      
      r = read (sock, msg, len);
#if 0
      if (r < 0)
	perror_fatal ("msg_recv:read");
#endif
      /* turn off the alarm */
      alarm (0);

      return r;
    }
  else
    /* time out */
    return 0;
}

static jmp_buf pipebroken_env;

static void
pipebroken (sig)
     int sig;
{
  signal (sig, SIG_IGN);

  signal (SIGPIPE, pipebroken);
  longjmp (pipebroken_env, 1);
}

static int
raw_send (void *msg, int len)
{
  int r;
  int index = 0;
  int sock = (role == ROLE_OF_SERVER) ? fs_id : s_id;

  signal (SIGPIPE, SIG_IGN);

  if (setjmp (pipebroken_env) == 0)
    {
      while (len > 0)
	{
	  r = write (sock, (char *)msg + index, len - index);

	  if (r < 0)
#if 0
	    perror_fatal ("msg_send:write");
#else
	    break;
#endif
	  else
	    {
	      index += r;
	      len -= r;
	    }
	}
    }
  else
    r = -1;

/*  signal (SIGPIPE, SIG_DFL);*/

  return r;
}
