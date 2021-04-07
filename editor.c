#include "editor.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#define TAB_STOP 4

void editorUpdateRow(erow *row)
{
  int tabs = 0;
  int j;
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') tabs++;
  }

  free(row->render);
  row->render = malloc(row->size + tabs * (TAB_STOP - 1) + 1);

  int idx = 0;
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % TAB_STOP != 0) row->render[idx++] = ' ';
    } else {
      row->render[idx++] = row->chars[j];
    }
  }
  row->render[idx] = '\0';
  row->rsize = idx;
}

void editorAppendRow(char *s, size_t len)
{
  // Increasing the size of the rows array
  E.rows = realloc(E.rows, sizeof(erow) * (E.numRows + 1));

  int i = E.numRows;

  // Add the new row
  E.rows[i].size = len;
  E.rows[i].chars = malloc(len + 1);
  memcpy(E.rows[i].chars, s, len);
  E.rows[i].chars[len] = '\0';

  E.rows[i].rsize = 0;
  E.rows[i].render = NULL;
  editorUpdateRow(&E.rows[i]);

  E.numRows++;
}

void editorOpen(char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (!fp) die("fopen");

  char *line = NULL;
  size_t lineCap = 0;
  ssize_t lineLen;

  while ((lineLen = getline(&line, &lineCap, fp)) != -1) {
    while (lineLen > 0
           && (line[lineLen - 1] == '\n' || line[lineLen - 1] == '\r'))
      lineLen--;

    editorAppendRow(line, lineLen);
  }
  free(line);
  fclose(fp);
}

void editorScroll()
{
  int fileRow = E.cy + E.rowOff;
  if (E.cy == E.editorRows) {
    E.cy--;
    if (fileRow < E.numRows) {
      E.rowOff++;
    }
  } else if (E.cy < 0) {
    E.cy++;
    if (E.rowOff != 0) {
      E.rowOff--;
    }
  }
}

void editorSetStatusMessage(char *msg)
{
  E.statusMsg = msg;
}

void editorDrawStatusBar(struct abuf *ab)
{
  char status[E.screenCols];
  int statusLen = snprintf(
      status, sizeof(status),
      "rowOff: %d | numRows: %d | cx: %d | cy: %d | editorRows: %d | "
      "screenRows: %d | rowSize: %d | statusMessage: %s",
      E.rowOff, E.numRows, E.cx, E.cy, E.editorRows, E.screenRows, E.rows[E.cy].size, E.statusMsg);
      abAppend(ab, status, statusLen);
}

// Draw what is supposed to be shown on screen atm
void editorDrawRows(struct abuf *ab)
{
  int y;
  for (y = 0; y < E.editorRows; y++) {
    int fileRow = y + E.rowOff;
    if (fileRow >= E.numRows) {
      if (y >= E.numRows) {
        abAppend(ab, "~", 1);
      }
    } else {
      int len = E.rows[fileRow].rsize;
      if (len > E.screenCols) len = E.screenCols;
      abAppend(ab, E.rows[fileRow].render, len);
    }

    // Clear to the right of the cursor
    abAppend(ab, "\x1b[K", 3);
    abAppend(ab, "\r\n", 2);
  }

  editorDrawStatusBar(ab);
}

// Clear the screen
void editorRefreshScreen()
{
  editorScroll();

  struct abuf ab = ABUF_INIT;

  // Hide cursor
  abAppend(&ab, "\x1b[?25l", 6);
  // Move cursor to top
  abAppend(&ab, "\x1b[H", 3);

  // Append rows to appendbuffer
  editorDrawRows(&ab);

  // Put the cursor in the correct position
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
  abAppend(&ab, buf, strlen(buf));

  // Show cursor
  abAppend(&ab, "\x1b[?25h", 6);

  // Write from appendbuffer and free its memory
  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}

void editorMoveCursor(char key)
{
  switch (key) {
    case 'h':
      if (E.cx != 0) E.cx--;
      break;
    case 'j':
      if (E.cy < E.editorRows && E.cy < E.numRows - 1) {
        E.cy++;
      }
      break;
    case 'k':
      if (E.cy >= 0) {
        E.cy--;
      }
      break;
    case 'l':
      if (E.cx != E.screenCols - 1) E.cx++;
      break;
  }
}

// Read a key from stdin
char editorReadKey()
{
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

// Handle keypresses
void editorProcessKeypress()
{
  char c = editorReadKey();

  switch (c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
    case 'h':
    case 'j':
    case 'k':
    case 'l':
      editorMoveCursor(c);
      break;
  }
}

void initEditor()
{
  E.cx = 0;
  E.cy = 0;
  E.rowOff = 0;
  E.numRows = 0;
  E.rows = NULL;
  E.statusMsg = NULL;

  if (getWindowSize(&E.screenRows, &E.screenCols) == -1) {
    die("getWindowSize");
  }

  // To allow for my status bar at the bottom
  E.editorRows = E.screenRows - 1;
}
