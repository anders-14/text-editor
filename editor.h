#ifndef EDITOR_H
#define EDITOR_H

void initEditor();
void editorOpenFile(char *filename);
void editorOpenEmptyBuffer();
void editorRefreshScreen();
void editorProcessKeypress();

#endif
