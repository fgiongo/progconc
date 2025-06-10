#include "utils.h"

float maxf(float *f, int len) {
    if (len < 1) {
        return 0.0;
    }
    float max = f[0];
    for (int i = 0; i < len; ++i) {
        max = (f[i] > max) ? f[i] : max;
    }
    return max;
}
