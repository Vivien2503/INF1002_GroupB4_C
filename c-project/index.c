/*
 *This file contains the fast lookup functionality.
 *It includes Fast Lookup: open-addressing hash index (ID -> array index)
*/


#include "student_db.h"

#define HSIZE 257

typedef struct {
    int key;
    int pos;
} Slot;

static Slot indexTable[HSIZE];

static unsigned hmix(unsigned x) {
    x ^= x >> 16; x *= 0x7feb352d;
    x ^= x >> 15; x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

static void index_clear(void) {
    for (int i = 0; i < HSIZE; i++) {
        indexTable[i].key = 0;
        indexTable[i].pos = -1;
    }
}

void index_build(const StudentRecord* recs, int count) {
    index_clear();
    for (int i = 0; i < count; i++) {
        unsigned h = hmix((unsigned)recs[i].id) % HSIZE;
        for (int step = 0; step < HSIZE; step++, h = (h + 1) % HSIZE) {
            if (indexTable[h].key == 0 || indexTable[h].key == recs[i].id) {
                indexTable[h].key = recs[i].id;
                indexTable[h].pos = i;
                break;
            }
        }
    }
}

int index_get(int id, int* out_pos) {
    unsigned h = hmix((unsigned)id) % HSIZE;
    for (int step = 0; step < HSIZE; step++, h = (h + 1) % HSIZE) {
        if (indexTable[h].key == 0) return 0;
        if (indexTable[h].key == id) {
            if (out_pos) *out_pos = indexTable[h].pos;
            return 1;
        }
    }
    return 0;
}

void index_put(int id, int pos) {
    unsigned h = hmix((unsigned)id) % HSIZE;
    for (int step = 0; step < HSIZE; step++, h = (h + 1) % HSIZE) {
        if (indexTable[h].key == 0 || indexTable[h].key == id) {
            indexTable[h].key = id;
            indexTable[h].pos = pos;
            return;
        }
    }
}

void index_rebuild(const StudentRecord* recs, int count) {
    index_build(recs, count);
}
