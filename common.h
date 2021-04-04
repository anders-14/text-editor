#pragma once

#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

void die(const char *s);
void disableRawMode();
void enableRawMode();

struct abuf {
  char *b;
  int len;
};

#define ABUF_INIT                                                              \
  { NULL, 0 }

void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);
int getCursorPosition(int *rows, int *cols);
int getWindowSize(int *rows, int *cols);
