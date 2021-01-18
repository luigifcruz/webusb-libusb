#ifndef AUDIOCONTEXT_H
#define AUDIOCONTEXT_H

#include <cstddef>

typedef struct {
  int sample_rate;
} audiocontext_config;

void audiocontext_init(audiocontext_config*);

void audiocontext_feed(float**, size_t, size_t, int);

void audiocontext_close();

#endif
