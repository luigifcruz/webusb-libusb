#include <iostream>
#include <vector>

#include <emscripten.h>
#include <audiocontext.h>

int main(int argc, char **argv) {
  std::cout << "Click inside the DOM in the next 1s to enable sound." << std::endl;

  emscripten_sleep(1000);

  std::vector<float> samples(44100);

  int i = 0;
  for (auto& sample : samples) {
      sample = (float)i++;
      if (i > 100)
        i = 0;
  }

  std::vector<float*> channels = {
      samples.data(),
  };

  audiocontext_config cfg{};
  cfg.sample_rate = 44100;
  audiocontext_init(&cfg);

  audiocontext_feed(channels.data(), channels.size(), samples.size(), 44100);

  std::cout << "Now playing..." << std::endl;
  emscripten_sleep(1000);

  audiocontext_close();

  printf("AUDIO CONTEXT SUCCESSFUL\n");

  return 0;
}
