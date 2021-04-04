#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#define EDITOR_VERSION "0.0.1"
#define CTRL_KEY(k) ((k)&0x1f)

typedef struct erow {
  int size;
  char *chars;
} erow;

struct editorConfig {
  int cx, cy;
  int rowOff;
  int editorRows;
  int screenRows;
  int screenCols;
  int numRows;
  erow *row;
  char *statusMsg;
} E;

void editorAppendRow(char *s, size_t len);
void editorOpen(char *filename);
void editorScroll();
void editorSetStatusMessage(char *msg);
void editorDrawStatusBar(struct abuf *ab);
void editorDrawRows(struct abuf *ab);
void editorRefreshScreen();
void editorMoveCursor(char key);
char editorReadKey();
void editorProcessKeypress();
void initEditor();
