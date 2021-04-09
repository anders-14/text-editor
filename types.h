#ifndef TYPES_H
#define TYPES_H

typedef struct {
  char *chars;
  int size;
  char *render;
  int rsize;
} erow;

typedef struct {
  int screenX;
  int screenY;
  int fileX;
  int fileY;
} cursor;

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
