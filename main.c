#include <stdlib.h>

#include "common.h"
#include "draw.h"
#include "file.h"
#include "input.h"

void initEditor(editorConfig *E, cursor *c)
{
  E->cursor = c;
  E->insertMode = 0;
  E->numRows = 0;
  E->rows = NULL;
  E->statusMsg = NULL;
  E->filename = NULL;

  if (getWindowSize(&E->screenRows, &E->screenCols) == -1) {
    die("getWindowSize");
  }

  E->editorRows = E->screenRows - 1;
}

int main(int argc, char *argv[])
{
  enableRawMode();

  editorConfig E;
  cursor c = {0};
  initEditor(&E, &c);

  if (argc >= 2) {
    openFile(&E, argv[1]);
  } else {
    openEmptyFile(&E);
  }

  while (1) {
    drawScreen(&E);
    processKeyboardInput(&E);
  }

  return 0;
}
