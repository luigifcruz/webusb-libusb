#ifndef AUDIOCONTEXT_H
#define AUDIOCONTEXT_H

#include <cstddef>

typedef struct {
  int sample_rate;
} audiocontext_config;

void audiocontext_init(audiocontext_config*);

void audiocontext_feed(float**, int, int, int);

void audiocontext_close();

#endif
