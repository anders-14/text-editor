#include "draw.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

void drawStatusBar(editorConfig *E, abuf *ab)
{
  char status[E->screenCols];
  int statusLen
      = snprintf(status, sizeof(status),
                 "sx: %d | sy: %d | fx: %d | fy: %d | sc: %d | sr: %d | nr: %d "
                 "| rs: %d | %s",
                 E->cursor->screenX, E->cursor->screenY, E->cursor->fileX,
                 E->cursor->fileY, E->screenCols, E->screenRows, E->numRows,
                 E->rows[E->cursor->fileY].rsize, E->statusMsg);
  abAppend(ab, status, statusLen);
  // Clear to the right of the cursor
  abAppend(ab, "\x1b[K", 3);
}

void drawPrompt(editorConfig *E, abuf *ab)
{
  abAppend(ab, ":", 1);
  if (E->promptValue) {
    abAppend(ab, E->promptValue, strlen(E->promptValue));
  }
  abAppend(ab, "\x1b[K", 3);
}

void drawRows(editorConfig *E, abuf *ab)
{
  int i;
  for (i = 0; i < E->editorRows; i++) {
    if (i >= E->numRows) {
      abAppend(ab, "~", 1);
    } else {
      int offset = E->cursor->fileY - E->cursor->screenY;
      erow *row = &E->rows[offset + i];
      abAppend(ab, row->render, row->rsize);
    }
    abAppend(ab, "\x1b[K", 3);
    abAppend(ab, "\r\n", 2);
  }
}

void drawCursor(abuf *ab, int x, int y)
{
  char buf[32];
  sprintf(buf, "\x1b[%d;%dH", y, x);
  abAppend(ab, buf, strlen(buf));
}

void drawScreen(editorConfig *E)
{
  // Create a new append buffer
  abuf ab = {NULL, 0};

  // Hide cursor during redraw
  abAppend(&ab, "\x1b[?25l", 6);

  // Move cursor to top
  abAppend(&ab, "\x1b[H", 3);

  // Append rows to appendbuffer
  drawRows(E, &ab);

  if (E->promptActive == 1) {
    drawPrompt(E, &ab);
    drawCursor(&ab, (E->promptValue ? strlen(E->promptValue) : 0) + 2, E->screenRows);
  } else {
    drawStatusBar(E, &ab);
    drawCursor(&ab, E->cursor->screenX + 1, E->cursor->screenY + 1);
  }


  // Show cursor
  abAppend(&ab, "\x1b[?25h", 6);

  // Write from appendbuffer and free its memory
  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}
