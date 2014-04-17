#ifndef __RANDOM_H
#define __RANDOM_H

typedef struct random Random;

struct random {
  unsigned int seed;
};

#define RAND_SEED(r)	((r)->seed)

#define RAND_INT(r, n)	(rand_random (r) % (n))
#define RAND_ONEIN(r, n) (RAND_INT (r, n) == 0)

 /* prototypes */

Random *rand_new (void);
Random *rand_init (Random *r);
void rand_free (Random *r);

Random *rand_default (void);

unsigned int rand_srand (Random *r, unsigned int seed);
unsigned int rand_random (Random *r);

#endif /* __RANDOM_H */
