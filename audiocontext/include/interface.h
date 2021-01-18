#include "audiocontext.h"

void _audiocontext_init(audiocontext_config*);

void _audiocontext_feed(float**, size_t, size_t, int);

void _audiocontext_close();
