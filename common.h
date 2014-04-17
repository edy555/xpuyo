#ifndef __COMMON_H
#define __COMMON_H

/* boolean */

typedef char boolean;

#define TRUE 1
#define FALSE 0


/* allocation */

void *xmalloc (unsigned size);
void *xcalloc (unsigned siz1, unsigned siz2);
void xfree (void *p);

/* error */

/*volatile*/ void fatal (char *msg);


#endif /* __COMMON_H */
