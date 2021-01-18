#include <iostream>

#include <emscripten/threading.h>

#include "audiocontext.h"
#include "interface.h"

void audiocontext_init(audiocontext_config* config) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VI, _audiocontext_init,
        config);
}

void audiocontext_feed(float** samples, size_t ch_n, size_t len, int fs) {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VIIII, _audiocontext_feed,
        samples, ch_n, len, fs);
}

void audiocontext_close() {
#ifdef DEBUG_TRACE
    std::cout << "> " << __func__ << std::endl;
#endif
    emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_V, _audiocontext_close);
}
