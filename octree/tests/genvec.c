#include <stdlib.h>
#include "genvec.h"

Vec3 *genRandomPoints(int nPoints, float min, float max) {
    if (min > max) return NULL;

    Vec3 *arr = malloc(nPoints * sizeof(Vec3));
    if (!arr) return NULL;

    float range = max - min;
    for (int i = 0; i < nPoints; i++) {
        float r1 = (float)rand() / (float)RAND_MAX;
        float r2 = (float)rand() / (float)RAND_MAX;
        float r3 = (float)rand() / (float)RAND_MAX;
        arr[i].x = min + r1 * range;
        arr[i].y = min + r2 * range;
        arr[i].z = min + r3 * range;
    }

    return arr;
}
