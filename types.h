#ifndef TYPES_H
#define TYPES_H

typedef struct {
  int size;
  int rsize;
  char *chars;
  char *render;
} erow;

typedef struct {
  int cx, cy;
  int rowOff;
  int colOff;
  int editorRows;
  int screenRows;
  int screenCols;
  int numRows;
  erow *rows;
  char *statusMsg;
} editorConfig;

typedef struct {
  char *b;
  int len;
} abuf;

#endif
