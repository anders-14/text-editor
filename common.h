#ifndef COMMON_H
#define COMMON_H

#include "types.h"

void abAppend(abuf *ab, const char *s, int len);
void abFree(abuf *ab);

int getWindowSize(int *rows, int *cols);

void die(const char *s);

void enableRawMode();

#endif
