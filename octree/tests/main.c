#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../octree.h"
#include "genvec.h"

Vec3 *genRandomPoints(int nPoints, float min, float max);

#define OCT_DEPTH 8

int main(void){
    srand((unsigned)time(NULL));
    int nPoints = 1000000;
    float max = 1000.0;
    float min = -1000.0;
    Vec3 *randomPoints = genRandomPoints(nPoints, min, max);
    Octree *octree = NewOctree(randomPoints, nPoints, OCT_DEPTH, 20);
    fprintf(stdout, "Octree cube:(%f, %f, %f), (%f, %f, %f)\n",
            octree->cube[0].x, octree->cube[0].y, octree->cube[0].z,
            octree->cube[1].x, octree->cube[1].y, octree->cube[1].z);
    return 0;
}

