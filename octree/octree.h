#ifndef OCT_H
#define OCT_H

#define OCT_SIZE (16777216)

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef struct _octreeNode {
    int isEmpty;
    Vec3 *points;
} OctNode;

typedef struct {
    int depth;
    Vec3 bBox[2];
    OctNode *buffer;
} Octree;


Octree *NewOctree(Vec3 *vecs, int len, int depth, int nThreads);
void *findBoundingBox(void *targ);

#endif
