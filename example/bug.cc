#include <iostream>
#include <math.h>
#include <vector>

#include <pthread.h>
#include <emscripten.h>
#include <emscripten/threading.h>
#include <emscripten/bind.h>

using namespace emscripten;

static _Atomic bool started = false;

void *WUSBThread(void*) {
    started = true;
    emscripten_exit_with_live_runtime();
    return NULL;
}

void random_func(int* a) {
    std::cout << "Hello I'm thread: " << *a << std::endl;
}

void memory_func(float* samples, size_t len) {
    val AudioContext = val::global("AudioContext");
    val cfg = val::object();
    cfg.set("sampleRate", 48000);
    val context = AudioContext.new_(cfg);
    val::global().set("audioContext", context);

    val buf = context.call<val>("createBuffer", 1, len, 48000);
    auto out = val(typed_memory_view(len, samples));
    buf.call<val>("getChannelData", 0).call<void>("set", out);

    val src = context.call<val>("createBufferSource");
    src.set("buffer", buf);
    src.call<void>("connect", context["destination"]);

    src.call<void>("start");
}

int main(int argc, char **argv) {
    pthread_t thread;

    if (pthread_create(&thread, NULL, WUSBThread, nullptr) != 0)
        return 1;

    while (!started)
        emscripten_sleep(100);

    int hello = 42;

    emscripten_dispatch_to_thread_sync(thread, EM_FUNC_SIG_VI, random_func, nullptr, &hello);

    emscripten_sleep(1000);

    size_t len = 48000;
    auto samples = (float*)malloc(sizeof(float)*len);

    emscripten_sync_run_in_main_runtime_thread(EM_FUNC_SIG_VII, memory_func, samples, len);

    std::cout << "finished" << std::endl;

    return 0;
}

