#ifndef FILE_H
#define FILE_H

#include "types.h"

void openFile(editorConfig *E, char *filename);
void openEmptyFile(editorConfig *E);
void saveFile(editorConfig *E);

#endif
