#include "editor.h"

void editorAppendRow(char *s, size_t len)
{
  E.row = realloc(E.row, sizeof(erow) * (E.numRows + 1));

  int at = E.numRows;

  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';
  E.numRows++;
}

void editorOpen(char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (!fp)
    die("fopen");

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
  char status[120];
  int statusLen = snprintf(
      status, sizeof(status),
      "rowOff: %d | numRows: %d | cx: %d | cy: %d | editorRows: %d | "
      "screenRows: %d | statusMessage: %s",
      E.rowOff, E.numRows, E.cx, E.cy, E.editorRows, E.screenRows, E.statusMsg);
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
      int len = E.row[fileRow].size;
      if (len > E.screenCols)
        len = E.screenCols;
      abAppend(ab, E.row[fileRow].chars, len);
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
      if (E.cx != 0)
        E.cx--;
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
      if (E.cx != E.screenCols - 1)
        E.cx++;
      break;
  }
}

// Read a key from stdin
char editorReadKey()
{
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
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
  E.row = NULL;
  E.statusMsg = NULL;

  if (getWindowSize(&E.screenRows, &E.screenCols) == -1) {
    die("getWindowSize");
  }

  // To allow for my status bar at the bottom
  E.editorRows = E.screenRows - 1;
}
