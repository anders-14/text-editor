#ifndef COMMON_H
#define COMMON_H

struct abuf {
  char *b;
  int len;
};

#define ABUF_INIT                                                              \
  {                                                                            \
    NULL, 0                                                                    \
  }

void abAppend(struct abuf *ab, const char *s, int len);
void abFree(struct abuf *ab);

int getWindowSize(int *rows, int *cols);

void die(const char *s);

void enableRawMode();

#endif
