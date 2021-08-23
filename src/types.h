#ifndef TYPES_H
#define TYPES_H

enum modes {
  NORMAL,
  INSERT,
  PROMPT,
};

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
  char *filename;
  int editorRows;
  int screenRows;
  int screenCols;
  int numRows;
  erow *rows;
  cursor *cursor;
  char *statusMsg;
  char *promptValue;
  enum modes mode;
} editorConfig;

typedef struct {
  char *b;
  int len;
} abuf;

#endif
