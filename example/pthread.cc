#include <iostream>
#include <cstdlib>
#include <pthread.h>

#define NUM_THREADS 1

void *PrintHello(void *threadid) {
   long tid;
   tid = (long)threadid;
   std::cout << "Hello World! Thread ID, " << tid << std::endl;
   pthread_exit(NULL);
}

int main() {
    std::cout << "Hello from WASM C++." << std::endl;

#ifdef __EMSCRIPTEN_PTHREADS__
    std::cout << "pthreads enabled" << std::endl;
#endif

    pthread_t threads[NUM_THREADS];
    int rc;
    int i;

    for( i = 0; i < NUM_THREADS; i++ ) {
        std::cout << "main() : creating thread, " << i << std::endl;
        rc = pthread_create(&threads[i], NULL, PrintHello, (void *)i);
    }
    pthread_exit(NULL);

    return 0;
}
