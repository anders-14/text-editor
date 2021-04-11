#include "insert.h"

#include <stdlib.h>
#include <string.h>

void insertCharInRowAtIndex(erow *row, int idx, int c)
{
  // If the index is out of range dont do anything
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
