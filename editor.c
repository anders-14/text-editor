#include "editor.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "cursor.h"
#include "insert.h"
#include "types.h"

#define TAB_STOP 4

#define CTRL_KEY(k) ((k)&0x1f)

editorConfig E;

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

void editorAppendRow(char *line, size_t len)
{
  // Increase the size of E.rows to allow for a new row to be added
  E.rows = realloc(E.rows, sizeof(erow) * (E.numRows + 1));

  // Pointer to new row
  erow *row = &E.rows[E.numRows];

  // Add the new row
  row->size = len;
  row->chars = malloc(len + 1);
  memcpy(row->chars, line, len);
  row->chars[len] = '\0';

  // Rendered row will have different length due to escape chars and more
  row->rsize = 0;
  row->render = NULL;
  editorUpdateRow(row);

  E.numRows++;
}

void editorInsertChar(int c)
{
  if (E.cursor.fileY == E.numRows) {
    editorAppendRow("", 0);
  }
  insertCharInRowAtIndex(&E.rows[E.cursor.fileY], E.cursor.fileX, c);
  editorUpdateRow(&E.rows[E.cursor.fileY]);
  E.cursor.screenX++;
  E.cursor.fileX++;
}

void editorOpenFile(char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (!fp) die("fopen");

  char *line = NULL;
  size_t lineSize = 0;
  ssize_t lineLen;

  while ((lineLen = getline(&line, &lineSize, fp)) != -1) {
    // Remove newline characters
    while (lineLen > 0
           && (line[lineLen - 1] == '\n' || line[lineLen - 1] == '\r')) {
      lineLen--;
    }
    editorAppendRow(line, lineLen);
  }

  free(line);
  fclose(fp);
}

void editorDrawStatusBar(abuf *ab)
{
  char status[E.screenCols];
  int statusLen = snprintf(
      status, sizeof(status),
      "sx: %d | sy: %d | fx: %d | fy: %d | sc: %d | sr: %d | nr: %d | rs: %d",
      E.cursor.screenX, E.cursor.screenY, E.cursor.fileX, E.cursor.fileY,
      E.screenCols, E.screenRows, E.numRows, E.rows[E.cursor.fileY].rsize);
  abAppend(ab, status, statusLen);
  // Clear to the right of the cursor
  abAppend(ab, "\x1b[K", 3);
}

void editorDrawRows(abuf *ab)
{
  int i;
  for (i = 0; i < E.editorRows; i++) {
    if (i >= E.numRows) {
      abAppend(ab, "~", 1);
    } else {
      int offset = E.cursor.fileY - E.cursor.screenY;
      erow *row = &E.rows[offset + i];
      abAppend(ab, row->render, row->rsize);
    }
    abAppend(ab, "\x1b[K", 3);
    abAppend(ab, "\r\n", 2);
  }

  editorDrawStatusBar(ab);
}

void editorRefreshScreen()
{
  // Create a new append buffer
  abuf ab = {NULL, 0};

  // Hide cursor during redraw
  abAppend(&ab, "\x1b[?25l", 6);

  // Move cursor to top
  abAppend(&ab, "\x1b[H", 3);

  // Append rows to appendbuffer
  editorDrawRows(&ab);

  // Put the cursor in the correct position
  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cursor.screenY + 1,
           E.cursor.screenX + 1);
  abAppend(&ab, buf, strlen(buf));

  // Show cursor
  abAppend(&ab, "\x1b[?25h", 6);

  // Write from appendbuffer and free its memory
  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
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

  if (E.insertMode) {
    switch (c) {
      case CTRL_KEY('c'):
        E.insertMode = 0;
        break;
      default:
        editorInsertChar(c);
        break;
    }
  } else {
    switch (c) {
      case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
      case 'i':
        E.insertMode = 1;
        break;
      case 'h':
      case 'j':
      case 'k':
      case 'l':
      case 'g':
      case 'G':
        cursorMove(c, &E);
        break;
    }
  }
}

void initEditor()
{
  cursor c = {0};
  E.cursor = c;
  E.insertMode = 0;
  E.numRows = 0;
  E.rows = NULL;
  E.statusMsg = NULL;

  if (getWindowSize(&E.screenRows, &E.screenCols) == -1) {
    die("getWindowSize");
  }

  // To allow for my status bar at the bottom
  E.editorRows = E.screenRows - 1;
}
