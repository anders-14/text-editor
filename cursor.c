#include <stdio.h>
#include "cursor.h"

int cursorHasValidPositionX(editorConfig *E)
{
  cursor c = *E->cursor;
  int rowLen = E->rows[c.fileY].rsize;
  return c.fileX;
}

/* int cursorHasValidPositionY(editorConfig *E) */
/* { */
/*   /1* cursor c = *E->cursor; *1/ */
/*   return 0; */
/* } */

