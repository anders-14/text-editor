#include "file.h"

#include <stdio.h>

#include "common.h"
#include "edit.h"

void openFile(editorConfig *E, char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (!fp) die("fopen");

  char *line = NULL;
  size_t lineSize = 0;
  ssize_t lineLen;

  while ((lineLen = getline(&line, &lineSize, fp)) != -1) {
    // Remove newline characters
    while (lineLen > 0
           && (line[lineLen - 1] == '\n' || line[lineLen - 1] == '\r')) {
      lineLen--;
    }
    appendRow(E, line, lineLen);
  }

  E->filename = filename;

  free(line);
  fclose(fp);
}

void openEmptyFile(editorConfig *E)
{
  appendRow(E, "", 0);
}
