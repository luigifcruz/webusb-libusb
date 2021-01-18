#include <iostream>
#include <math.h>
#include <vector>

#include <emscripten.h>
#include <audiocontext.h>

int main(int argc, char **argv) {
  std::cout << "Click inside the DOM in the next 1s to enable sound." << std::endl;

  emscripten_sleep(1000);

  std::vector<float> samples(48000);

  for (int i = 0; i < samples.size(); i++)
      samples[i] = sin(2*M_PI*15e3/48000*i);

  std::vector<float*> channels = {
      samples.data(),
  };

  audiocontext_config cfg{};
  cfg.sample_rate = 48000;
  audiocontext_init(&cfg);

  for (int i = 0; i < 10; i++) {
    audiocontext_feed(channels.data(), channels.size(), samples.size(), 48000);
    emscripten_sleep(1000);
  }

  audiocontext_close();

  printf("AUDIO CONTEXT SUCCESSFUL\n");

  return 0;
}
