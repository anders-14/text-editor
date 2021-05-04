#ifndef EDITOR_H
#define EDITOR_H

#include <stdlib.h>

#include "types.h"

void insertRowAtIndex(editorConfig *E, int idx, char *s, size_t len);
void appendRow(editorConfig *E, char *line, size_t len);
void insertChar(editorConfig *E, int c);
void deleteCharBeforeCursor(editorConfig *E);

#endif
