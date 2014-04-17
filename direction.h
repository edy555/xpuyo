#ifndef __DIRECTION_H
#define __DIRECTION_H

 /* constants */

enum direction {
  DIR_UPPER, DIR_RIGHT, DIR_LOWER, DIR_LEFT,
  DIR_DIRECTIONS
};

typedef enum direction Direction;

#define DIR_MASK(dir)		(1 << (dir))


 /* methods */

#define DIR_ROTATE_RIGHT(dir)	(((dir) + 1) & (DIR_DIRECTIONS - 1))
#define DIR_ROTATE_LEFT(dir)	(((dir) - 1) & (DIR_DIRECTIONS - 1))

extern short dir_offset_table[2][DIR_DIRECTIONS];

#define DIR_OFFSET_X(dir)	(dir_offset_table[0][(dir)])
#define DIR_OFFSET_Y(dir)	(dir_offset_table[1][(dir)])


#define DIR_DIVISION	4

float dir_fine_offset_x (Direction dir, int subdir);
float dir_fine_offset_y (Direction dir, int subdir);

#endif /* __DIRECTION_H */
