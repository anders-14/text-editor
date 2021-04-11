#include "edit.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "cursor.h"
#include "types.h"

#define TAB_STOP 4


void insertCharInRowAtIndex(erow *row, int idx, int c)
{
  if (idx < 0 || idx > row->size) return;

  row->chars = realloc(row->chars, row->size + 2);
  memmove(&row->chars[idx + 1], &row->chars[idx], row->size - idx + 1);
  row->size++;
  row->chars[idx] = c;
}

void deleteCharInRowAtIndex(erow *row, int idx)
{
  if (idx < 0 || idx > row->size) return;

  memmove(&row->chars[idx], &row->chars[idx + 1], row->size - idx);
  row->size--;
}

void editorUpdateRow(erow *row)
{
  int tabs = 0;
  int j;
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') tabs++;
  }

  free(row->render);
  row->render = malloc(row->size + tabs * (TAB_STOP - 1) + 1);

  int idx = 0;
  for (j = 0; j < row->size; j++) {
    if (row->chars[j] == '\t') {
      row->render[idx++] = ' ';
      while (idx % TAB_STOP != 0) row->render[idx++] = ' ';
    } else {
      row->render[idx++] = row->chars[j];
    }
  }
  row->render[idx] = '\0';
  row->rsize = idx;
}

void appendRow(editorConfig *E, char *line, size_t len)
{
  // Increase the size of E->rows to allow for a new row to be added
  E->rows = realloc(E->rows, sizeof(erow) * (E->numRows + 1));

  // Pointer to new row
  erow *row = &E->rows[E->numRows];

  // Add the new row
  row->size = len;
  row->chars = malloc(len + 1);
  memcpy(row->chars, line, len);
  row->chars[len] = '\0';

  // Rendered row will have different length due to escape chars and more
  row->rsize = 0;
  row->render = NULL;
  editorUpdateRow(row);

  E->numRows++;
}

void insertChar(editorConfig *E, int c)
{
  if (E->cursor->fileY == E->numRows) {
    appendRow(E, "", 0);
  }
  insertCharInRowAtIndex(&E->rows[E->cursor->fileY], E->cursor->fileX, c);
  editorUpdateRow(&E->rows[E->cursor->fileY]);
  E->cursor->screenX++;
  E->cursor->fileX++;
}

void deleteCharBeforeCursor(editorConfig *E)
{
  if (E->cursor->fileX == 0) return;

  erow *row = &E->rows[E->cursor->fileY];
  deleteCharInRowAtIndex(row, E->cursor->fileX - 1);
  editorUpdateRow(row);
  E->cursor->screenX--;
  E->cursor->fileX--;
}

