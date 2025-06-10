#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include "octree.h"
#include "utils.h"

typedef struct {
    Vec3 *vecs;
    int len;
} FindBoundingBoxArg;


typedef struct {
    Octree *octree;
    Vec3 *vecs;
    int len;
    unsigned int *addressRange;
} OctreeInsertArg;

Octree *NewOctree(Vec3 *vecs, int len, int depth, int nThreads) {
    Octree *oct = malloc(sizeof(Octree));
    if (!oct) {
        return NULL;
    }

    oct->depth = depth;

    // Precisamos de 3 bits para endereçar corretamente cada novo
    // nível de profundidade da árvore
    OctNode *buf = malloc((int)pow(2, depth * 3) * sizeof(OctNode));
    if (!buf) {
        free(oct);
        return NULL;
    }

    if (nThreads < 1) {
        return NULL;
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * nThreads);
    if (!threads) {
        free(buf);
        free(oct);
        return NULL;
    }

    int err, blockSize;
    for (int i = 0; i < nThreads; i++) {
        FindBoundingBoxArg *arg = malloc(sizeof(FindBoundingBoxArg));
        if (!arg) {
            free(buf);
            free(oct);
            free(threads);
            return NULL;
        }

        blockSize = len / nThreads; 
        arg->vecs = &vecs[i * blockSize];
        arg->len = blockSize;

        if (i == nThreads - 1) {
            arg->len = len - (i * blockSize);
        }

        err = pthread_create(&threads[i], NULL, findBoundingBox, arg);
        if (err != 0) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }

    void *tRet;
    Vec3 *bBox;
    Vec3 *vRet;
    err = pthread_join(threads[0], &tRet);
    if (err != 0) {
        fprintf(stderr, "Error joining thread\n");
        exit(1);
    }
    bBox = (Vec3*)tRet;
    for (int i = 1; i < nThreads; ++i) {
        err = pthread_join(threads[i], &tRet);
        if (err != 0) {
            fprintf(stderr, "Error joining thread\n");
            exit(1);
        }
        vRet = (Vec3*)tRet;
        bBox[0].x = (vRet[0].x < bBox[0].x) ? vRet[0].x : bBox[0].x;
        bBox[1].x = (vRet[1].x > bBox[1].x) ? vRet[1].x : bBox[1].x;

        bBox[0].y = (vRet[0].y < bBox[0].y) ? vRet[0].y : bBox[0].y;
        bBox[1].y = (vRet[1].y > bBox[1].y) ? vRet[1].y : bBox[1].y;

        bBox[0].z = (vRet[0].z < bBox[0].z) ? vRet[0].z : bBox[0].z;
        bBox[1].z = (vRet[1].z > bBox[1].z) ? vRet[1].z : bBox[1].z;

        free(vRet);
    }

    if (bBox[0].x == bBox[1].x ||
        bBox[0].y == bBox[1].y ||
        bBox[0].z == bBox[1].z) {
        // Cubo não existe, pontos estão em um plano;
        free(bBox);
        free(buf);
        free(oct);
        free(threads);

        return NULL;
    }

    oct->rect[0] = bBox[0];
    oct->rect[1] = bBox[1];

    free(bBox);
    free(threads);

    oct->buffer = buf;

    populateOctree(oct, vecs, len, nThreads);

    return oct;
}

// Retorna dois vértices que definem o bounding box mínimo
// (alinhado com os eixos x, y, z) que contém todos os pontos
void *findBoundingBox(void *targ) {
    FindBoundingBoxArg *arg = (FindBoundingBoxArg*)targ;
    Vec3 *vecs = arg->vecs; 
    int len = arg->len;
    free(targ);

    if (len <= 1) {
        // não existe
        return NULL;
    }

    Vec3 *bBox = malloc(sizeof(Vec3) * 2);
    if (!bBox) {
        return NULL;
    }

    bBox[0].x = bBox[1].x = vecs[0].x;
    bBox[0].y = bBox[1].y = vecs[0].y;
    bBox[0].z = bBox[1].z = vecs[0].z;

    float x, y, z;
    for (int i = 1; i < len; ++i) {
        x = vecs[i].x;
        y = vecs[i].y;
        z = vecs[i].z;

        bBox[0].x = (x < bBox[0].x) ? x : bBox[0].x;
        bBox[1].x = (x > bBox[1].x) ? x : bBox[1].x;

        bBox[0].y = (y < bBox[0].y) ? y : bBox[0].y;
        bBox[1].y = (y > bBox[1].y) ? y : bBox[1].y;

        bBox[0].z = (z < bBox[0].z) ? z : bBox[0].z;
        bBox[1].z = (z > bBox[1].z) ? z : bBox[1].z;
    }

    if (bBox[0].x == bBox[1].x ||
        bBox[0].y == bBox[1].y ||
        bBox[0].z == bBox[1].z) {
        // Box não existe, pontos estão em um plano;
        return NULL;
    }

    return (void*)bBox;
}

void populateOctree(Octree* oct, Vec3 *vecs, int len, int nThreads) {
    if (nThreads < 1) {
        return;
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * nThreads);
    if (!threads) {
        fprintf(stderr, "populateOctree: Error creating threads\n");
        exit(1);
    }

    OctreeInsertArg *args = malloc(sizeof(PopulateOctreeArg) * nThreads);
    if (!args) {
        fprintf(stderr, "populateOctree: Error creating threads\n");
        exit(1);
    }

    int err;
    for (int i = 0; i < nThreads; ++i) {
        args[i]->vecs = vecs;
        args[i]->len = len;
        args[i]->addressRange = calculateAddressRange(i, nThreads, oct->depth);
        err = pthread_create(&threads[i], NULL, OctreeInsert, args[i]);
        if (err != 0) {
            fprintf(stderr, "Error creating thread\n");
            exit(1);
        }
    }

}


// Divide os endereços entre os threads, de forma razoavelmente igual
unsigned int *calculateAddressRange(int thread, int nThreads, int maxDepth) {
    int threadDepth;
    int nSubtrees;
    // Calculando o numero de sub-arvores por profundidade, até encontrarmos
    // uma profundidade onde cada thread pode trabalhar com pelo menos uma
    // sub-árvore de forma isolada
    for (threadDepth = 0, threadDepth <= maxDepth; ++threadDepth) {
        nSubtrees = (int)pow(8, threadDepth);
        if (nSubtrees >= nThreads) {
            break;
        }
    }

    int nSubtreesPerThread = nSubtrees / nThreads;
    int remainder = nSubtrees % nThreads;

    unsigned int shift = sizeof(unsigned int) - threadDepth * 3;
    unsigned int rangeStart = thread * nSubtreesPerThread;
    unsigned int rangeEnd = rangeStart + nSubtreesPerThread;
    rangeStart <<= shift;
    rangeEnd <<= shift;
    rangeStart |= threadDepth * 3;
    rangeEnd |= threadDepth * 3;

    unsigned int *ret = malloc(sizeof(unsigned int) * 2);
    if (!ret) {
        return NULL;
    }

    ret[0] = rangeStart;
    ret[1] = rangeEnd;

    return ret;
}


void *OctreeInsert(void *targ) {
    OctreeInsertArg *arg = (OctreeInsertArg*)targ;

    unsigned int addr;
    for (int i = 0; i < arg->len; ++i) {
        addr = calculateAddress(arg->octree, vecs[i]);
        if (addrIsInRange(addr, arg->addressRange[0], arg->addressRange[1])) {
            OctreeInsertAtAddr(arg->octree, arg->vecs[i], addr);
        }
    }
    free(arg->addressRange);
    free(arg);
    return NULL;
}


unsigned int calculateAddress(Octree *octree, Vec3 v){
    float delta[3];
    delta[0] = octree->bBox[1].x - octree->bBox[0].x;
    delta[1] = octree->bBox[1].y - octree->bBox[0].y;
    delta[2] = octree->bBox[1].z - octree->bBox[0].z;
    float side = maxf(delta);
    Vec3 center = {
        octree->bBox[0].x + (side/2),
        octree->bBox[0].y + (side/2),
        octree->bBox[0].z + (side/2)
    }

    return calculateAddressRec(octree->depth, v, center, side);
}

unsigned int calculateAddressRec(int level, Vec3 v, Vec3 center, float size) {
    if (level <= 0) {
        return 0;
    }

    bitx = (v.x > mid.x) ? 1 : 0;
    bity = (v.y > mid.y) ? 1 : 0;
    bitz = (v.z > mid.z) ? 1 : 0;

    unsigned int addrHigh = (bitx << 2) | (bity << 1) | bitz;

    float size *= 0.5;
    float quarter = size * 0.5;

    center.x += (bitx) ? quarter : -quarter;
    center.y += (bity) ? quarter : -quarter;
    center.z += (bitz) ? quarter : -quarter;

    unsigned int addrLow = calculateAddressRec(level - 1, v, center, size);
    return (addrHigh << (3 * level - 1)) | addrLow;
}

