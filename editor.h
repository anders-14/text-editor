#ifndef EDITOR_H
#define EDITOR_H

#define CTRL_KEY(k) ((k)&0x1f)

typedef struct {
  int size;
  int rsize;
  char *chars;
  char *render;
} erow;

struct editorConfig {
  int cx, cy;
  int rowOff;
  int editorRows;
  int screenRows;
  int screenCols;
  int numRows;
  erow *rows;
  char *statusMsg;
} E;

void initEditor();
void editorOpen(char *filename);
void editorRefreshScreen();
void editorProcessKeypress();

#endif
