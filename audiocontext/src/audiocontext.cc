#include <iostream>

#include <emscripten.h>
#include <emscripten/val.h>

#include "interface.h"

using namespace emscripten;

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
    val::global().set("audioTime", context["currentTime"].as<double>());
}

void _audiocontext_feed(float** samples, size_t ch_n, size_t len, int fs) {
    val context = val::global("audioContext");

    val buf = context.call<val>("createBuffer", ch_n, len, fs);
    for (int n = 0; n < ch_n; n++) {
        auto out = val(typed_memory_view(len, samples[n]));
        buf.call<val>("getChannelData", n).call<void>("set", out);
    }
    val src = context.call<val>("createBufferSource");
    src.set("buffer", buf);
    src.call<void>("connect", context["destination"]);

    double audioTime = val::global("audioTime").as<double>();
    double currentTime = context["currentTime"].as<double>();
    double duration = buf["duration"].as<double>();

    if (audioTime < currentTime) {
        audioTime = currentTime + 0.15;
    }

    src.call<void>("start", audioTime);
    val::global().set("audioTime", audioTime + buf["duration"].as<double>());
}

void _audiocontext_close() {
    val context = val::global("audioContext");
    context.call<void>("suspend");
    context.call<void>("close");
    val::global().set("audioContext", val::null());
}
