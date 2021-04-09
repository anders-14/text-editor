#include "common.h"
#include "editor.h"

int main(int argc, char *argv[])
{
  enableRawMode();
  initEditor();
  if (argc >= 2) editorOpenFile(argv[1]);

  while (1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
