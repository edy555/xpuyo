#include "direction.h"
#include "common.h"
#include "math.h"


#ifndef M_PI
#define M_PI 3.1415926535
#endif

short dir_offset_table[2][DIR_DIRECTIONS] = {
  { 0, 1, 0, -1}, { -1, 0, 1, 0}};

float dir_fine_offset_table[2][DIR_DIRECTIONS * DIR_DIVISION];

boolean table_initialized = FALSE;


static void
dir_init_table (void)
{
  int n;

  for (n = 0; n < DIR_DIRECTIONS * DIR_DIVISION; n++)
    {
      dir_fine_offset_table[0][n] = sin (M_PI / 180.
					 * (double) n * 90 / DIR_DIVISION);
      dir_fine_offset_table[1][n] = -cos (M_PI / 180.
					  * (double) n * 90 / DIR_DIVISION);
    }

  table_initialized = TRUE;
}

float
dir_fine_offset_x (Direction dir, int subdir)
{
  int d;

  if (!table_initialized)
    dir_init_table ();

  d = dir * DIR_DIVISION + subdir;
  d = d & (DIR_DIRECTIONS * DIR_DIVISION - 1);

  return dir_fine_offset_table[0][d];
}

float
dir_fine_offset_y (Direction dir, int subdir)
{
  int d;

  if (!table_initialized)
    dir_init_table ();

  d = dir * DIR_DIVISION + subdir;
  d = d & (DIR_DIRECTIONS * DIR_DIVISION - 1);

  return dir_fine_offset_table[1][d];
}



