#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define EDITOR_VERSION "0.0.1"

#define CTRL_KEY(k) ((k)&0x1f)

typedef struct erow {
  int size;
  char *chars;
} erow;

struct editorConfig {
  int cx, cy;
	int rowOff;
  int screenRows;
  int screenCols;
  int numRows;
  erow *row;
  struct termios orig_termios;
} E;

// Print error message and exit
void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  perror(s);
  exit(1);
}

// Return the terminal to its original state
void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = E.orig_termios;

  raw.c_iflag &= ~(ICRNL | IXON | BRKINT | INPCK | ISTRIP);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag &= ~(CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

// Read a key from stdin
char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }
  return c;
}

// Get terminal size through cursor pos at bottom right if
// getWindowSize fails
int getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
    return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }

  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[')
    return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
    return -1;

  return -1;
}

// Get terminal size in rows and cols
int getWindowSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      return -1;
    return getCursorPosition(rows, cols);
  } else {
    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
  }
}

void editorAppendRow(char *s, size_t len) {
  E.row = realloc(E.row, sizeof(erow) * (E.numRows + 1));

  int at = E.numRows;

  E.row[at].size = len;
  E.row[at].chars = malloc(len + 1);
  memcpy(E.row[at].chars, s, len);
  E.row[at].chars[len] = '\0';
  E.numRows++;
}

void editorOpen(char *filename) {
  FILE *fp = fopen(filename, "r");
  if (!fp)
    die("fopen");

  char *line = NULL;
  size_t lineCap = 0;
  ssize_t lineLen;
  lineLen = getline(&line, &lineCap, fp);

	while ((lineLen = getline(&line, &lineCap, fp)) != -1) {
    while (lineLen > 0 &&
           (line[lineLen - 1] == '\n' || line[lineLen - 1] == '\r'))
      lineLen--;

    editorAppendRow(line, lineLen);
  }
  free(line);
  fclose(fp);
}

struct abuf {
  char *b;
  int len;
};

#define ABUF_INIT                                                              \
  { NULL, 0 }

void abAppend(struct abuf *ab, const char *s, int len) {
  char *new = realloc(ab->b, ab->len + len);

  if (new == NULL)
    return;
  memcpy(&new[ab->len], s, len);
  ab->b = new;
  ab->len += len;
}

void abFree(struct abuf *ab) { free(ab->b); }

void editorScroll() {
	if (E.cy < E.rowOff) {
		E.rowOff = E.cy;
	}
	if (E.cy >= E.rowOff + E.screenRows) {
		E.rowOff = E.cy - E.screenRows + 1;
	}
}

// Draw tildes down the left hand side, like vim
void editorDrawRows(struct abuf *ab) {
  int y;
	for (y = 0; y < E.screenRows; y++) {
		int fileRow = y + E.rowOff;
		if (fileRow > E.numRows) {
			if (y >= E.numRows) {
				if (E.numRows == 0 && y == E.screenRows / 3) {
					char welcome[80];
					int welcomeLen = snprintf(welcome, sizeof(welcome),
						"Editor -- version %s", EDITOR_VERSION);
					if (welcomeLen > E.screenCols)
					welcomeLen = E.screenCols;

					int padding = (E.screenCols - welcomeLen) / 2;
					if (padding) {
						abAppend(ab, "~", 1);
						padding--;
					}
					while (padding--)
					abAppend(ab, " ", 1);
					abAppend(ab, welcome, welcomeLen);
				} else {
					abAppend(ab, "~", 1);
				}
			}
    } else {
      int len = E.row[fileRow].size;
      if (len > E.screenCols)
        len = E.screenCols;
      abAppend(ab, E.row[fileRow].chars, len);
    }

    // Clear to the right of the cursor
    abAppend(ab, "\x1b[K", 3);
    if (y < E.screenRows - 1)
      abAppend(ab, "\r\n", 2);
  }
}

// Clear the screen
void editorRefreshScreen() {
	editorScroll();

  struct abuf ab = ABUF_INIT;

  // Hide cursor
  abAppend(&ab, "\x1b[?25l", 6);
  // Move cursor to top
  abAppend(&ab, "\x1b[H", 3);

  editorDrawRows(&ab);

  char buf[32];
  snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
  abAppend(&ab, buf, strlen(buf));

  // Show cursor
  abAppend(&ab, "\x1b[?25h", 6);

  write(STDOUT_FILENO, ab.b, ab.len);
  abFree(&ab);
}

void editorMoveCursor(char key) {
  switch (key) {
  case 'h':
    if (E.cx != 0)
      E.cx--;
    break;
  case 'j':
    if (E.cy < E.numRows)
      E.cy++;
    break;
  case 'k':
    if (E.cy != 0)
      E.cy--;
    break;
  case 'l':
    if (E.cx != E.screenCols - 1)
      E.cx++;
    break;
  }
}

// Handle keypresses
void editorProcessKeypress() {
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

void initEditor() {
  E.cx = 0;
  E.cy = 0;
	E.rowOff = 0;
  E.numRows = 0;
  E.row = NULL;

  if (getWindowSize(&E.screenRows, &E.screenCols) == -1)
    die("getWindowSize");
}

int main(int argc, char *argv[]) {
  enableRawMode();
  initEditor();
  if (argc >= 2)
    editorOpen(argv[1]);

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
