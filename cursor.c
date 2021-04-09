#include "cursor.h"

#include <stdio.h>

int cursorCanMoveUp(editorConfig *E)
{
  cursor *c = &E->cursor;
  if (c->fileY == 0) return 0;
  return 1;
}

int cursorCanMoveDown(editorConfig *E)
{
  cursor *c = &E->cursor;
  if (c->fileY >= E->numRows - 1) return 0;
  return 1;
}

int cursorCanMoveLeft(editorConfig *E)
{
  cursor *c = &E->cursor;
  if (c->fileX == 0) return 0;
  return 1;
}

int cursorCanMoveRight(editorConfig *E)
{
  cursor *c = &E->cursor;
  erow line = E->rows[c->fileY];
  if (c->fileX >= line.rsize - 1) return 0;
  return 1;
}

int cursorAtScreenTop(editorConfig *E)
{
  cursor *c = &E->cursor;
  if (c->screenY == 0) return 1;
  return 0;
}

int cursorAtScreenBottom(editorConfig *E)
{
  cursor *c = &E->cursor;
  if (c->screenY == E->editorRows - 1) return 1;
  return 0;
}

int cursorAtScreenLeft(editorConfig *E)
{
  cursor *c = &E->cursor;
  if (c->screenX == 0) return 1;
  return 0;
}

int cursorAtScreenRight(editorConfig *E)
{
  cursor *c = &E->cursor;
  if (c->screenX == E->screenCols - 1) return 1;
  return 0;
}

void cursorSetValidX(editorConfig *E)
{
  cursor *c = &E->cursor;
  erow line = E->rows[c->fileY];
  if (c->screenX >= line.rsize) {
    c->screenX = line.rsize;
    c->fileX = line.rsize;
    if (line.rsize != 0) {
      c->screenX--;
      c->fileX--;
    }
  }
}

void cursorMove(char key, editorConfig *E)
{
  cursor *c = &E->cursor;
  switch (key) {
    case 'h':
      if (cursorCanMoveLeft(E)) {
        if (!cursorAtScreenLeft(E)) {
          c->screenX--;
        }
        c->fileX--;
      }
      break;
    case 'j':
      if (cursorCanMoveDown(E)) {
        if (!cursorAtScreenBottom(E)) {
          c->screenY++;
        }
        c->fileY++;
        cursorSetValidX(E);
      }
      break;
    case 'k':
      if (cursorCanMoveUp(E)) {
        if (!cursorAtScreenTop(E)) {
          c->screenY--; 
        }
        c->fileY--;
        cursorSetValidX(E);
      }
      break;
    case 'l':
      if (cursorCanMoveRight(E)) {
        if (!cursorAtScreenRight(E)) {
          c->screenX++;
        }
        c->fileX++;
      }
      break;
  }
}
