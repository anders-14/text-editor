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

void keypressNormalMode(editorConfig *E, char c)
{
  switch (c) {
    case ':':
      E->mode = PROMPT;
      break;
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
    case CTRL_KEY('s'):
      saveFile(E);
      break;
    case 'i':
      E->mode = INSERT;
      break;
    case 'a':
      E->mode = INSERT;
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

void keypressInsertMode(editorConfig *E, char c)
{
  switch (c) {
    case CTRL_KEY('c'):
      E->mode = NORMAL;
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
}

void keypressPromptMode(editorConfig *E, char c)
{
  switch (c) {
    case CTRL_KEY('c'):
      E->mode = NORMAL;
      break;
    default:
      break;
  }
}

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

  if (E->mode == NORMAL) {
    keypressNormalMode(E, c);
  } else if (E->mode == INSERT) {
    keypressInsertMode(E, c);
  } else if (E->mode == PROMPT) {
    keypressPromptMode(E, c);
  }
}
