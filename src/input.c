#include "input.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "cursor.h"
#include "edit.h"
#include "file.h"

#define CTRL_KEY(k) ((k)&0x1f)

enum keys {
  ENTER = 13,
  BACKSPACE = 127,
};

// Read keyboard input from stdin
char readKey()
{
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}

// Handle keyboard input
void processKeyboardInput(editorConfig *E)
{
  char c = readKey();

  if (E->insertMode) {
    switch (c) {
      case CTRL_KEY('c'):
        E->insertMode = 0;
        cursorMove(E, 'h');
        break;
      case ENTER:
        insertRowAtIndex(E, E->cursor->fileY + 1, "", 0);
        cursorMove(E, 'j');
        break;
      case BACKSPACE:
        deleteCharBeforeCursor(E);
        break;
      default:
        insertChar(E, c);
        break;
    }
  } else {
    switch (c) {
      case CTRL_KEY('q'):
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        exit(0);
        break;
      case CTRL_KEY('s'):
        saveFile(E);
        break;
      case 'i':
        E->insertMode = 1;
        break;
      case 'a':
        E->insertMode = 1;
        E->cursor->screenX++;
        E->cursor->fileX++;
        break;
      case 'h':
      case 'j':
      case 'k':
      case 'l':
      case 'g':
      case 'G':
        cursorMove(E, c);
        break;
    }
  }
}
