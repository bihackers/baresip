#ifndef PPSDEVICE_H
#define PPSDEVICE_H
#include <re.h>
#include <rem.h>
#include <baresip.h>
#include <pthread.h>
struct vidsrc_st {
    const struct vidsrc *vs;  /* inheritance */
    pthread_t thread;
    bool run;
    struct vidsz sz;
    int sindex;
    vidsrc_frame_h *frameh;
    void *arg;
};

#endif