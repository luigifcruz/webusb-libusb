#include "audiocontext.h"

void _audiocontext_init(audiocontext_config*);

void _audiocontext_feed(float**, int, int, int);

void _audiocontext_close();
