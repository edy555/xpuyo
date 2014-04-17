#include "common.h"
#include <X11/Intrinsic.h>

void *
xmalloc (unsigned size)
{
  void *p = XtMalloc (size);

  return p;
}

void *
xcalloc (unsigned siz1, unsigned siz2)
{
  void *p = XtCalloc (siz1, siz2);

  return p;
}
  
void 
xfree (void *p)
{
  XtFree (p);
}
