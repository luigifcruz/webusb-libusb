#include <iostream>

#include <emscripten.h>
#include <emscripten/val.h>

#include "interface.h"

using namespace emscripten;

val create_out_buffer(float* buffer, size_t size) {
    auto tmp = val(typed_memory_view(size, buffer));
    auto buf = val::global("Float32Array").new_(size);
    buf.call<val>("set", tmp);
    return buf;
}

void _audiocontext_init(audiocontext_config* config) {
    val AudioContext = val::global("AudioContext");
    if (!AudioContext.as<bool>()) {
        printf("No global AudioContext, trying webkitAudioContext.\n");
        AudioContext = val::global("webkitAudioContext");
    }

    val cfg = val::object();
    cfg.set("sampleRate", config->sample_rate);

    val context = AudioContext.new_(cfg);
    val::global().set("audioContext", context);
    val::global().set("audioTime", context["currentTime"]);
}

void _audiocontext_feed(float** samples, size_t ch_n, size_t len, int fs) {
    val context = val::global("audioContext");

    val buf = context.call<val>("createBuffer", ch_n, len, fs);
    for (int n = 0; n < ch_n; n++) {
        val out = create_out_buffer(samples[n], len);
        buf.call<val>("getChannelData", n).call<val>("set", out);
    }
    val src = context.call<val>("createBufferSource");
    src.set("buffer", buf);
    src.call<void>("connect", context["destination"]);

    int audioTime = val::global("audioTime").as<int>();
    int currentTime = context["currentTime"].as<int>();

    if (audioTime < currentTime) {
        audioTime = currentTime + 0.15;
    }

    src.call<void>("start", audioTime);
    val::global().set("audioTime", audioTime + buf["duration"].as<int>());
}

void _audiocontext_close() {
    val context = val::global("audioContext");
    context.call<void>("suspend");
    context.call<void>("close");
    val::global().set("audioContext", val::null());
}
