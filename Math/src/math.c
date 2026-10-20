#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
int soma(int a, int b) {
    return a + b;
}

EMSCRIPTEN_KEEPALIVE
int fatorial(int n) {
    if (n <= 1) return 1;
    return n * fatorial(n - 1);
}