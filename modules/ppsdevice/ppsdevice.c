#include "ppsdevice.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ppcs.h"
/**
 * @defgroup ppsdevice ppsdevice
 *
 * Video source using PPCS/libPPCS_API PPCS_API
 *
 *
 * Example config:
 \verbatim
  video_source            ppsdevice,/tmp/testfile.txt
 \endverbatim
 */

static struct vidsrc *mod_avf;

static void destructor(void *arg)
{
    struct vidsrc_st *st = arg;

    if (st->run) {
        st->run = false;
        pthread_join(st->thread, NULL);
    }
}

static int alloc(struct vidsrc_st **stp, const struct vidsrc *vs,
        struct media_ctx **mctx, struct vidsrc_prm *prm,
        const struct vidsz *size, const char *fmt,
        const char *dev, vidsrc_frame_h *frameh,
        vidsrc_error_h *errorh, void *arg)
{
    struct vidsrc_st *st;
    int  err = 0;

    (void)mctx;
    (void)fmt;
    (void)errorh;

    if (!stp || !vs || !prm || !size || !frameh)
        return EINVAL;

    debug("ppsdevice: alloc dev='%s'\n", dev);

    st = mem_zalloc(sizeof(*st), destructor);
    if (!st)
        return ENOMEM;

    st->vs     = vs;
    st->sz     = *size;
    st->frameh = frameh;
    st->arg    = arg;
    st->run = true;
    err = connectIpc("uuid","password","initstring");
    if (err<0) {
        st->run = false;
        goto out;
    }
    err=startlive(st);
     if (err<0) {
        st->run = false;
        goto out;
    }
     err=0;
 out:
    if (err)
        mem_deref(st);
    else
        *stp = st;

    return err;
}

static int module_init(void)
{
    return vidsrc_register(&mod_avf, baresip_vidsrcl(), "ppsdevice", alloc, NULL);
}


static int module_close(void)
{
    mod_avf = mem_deref(mod_avf);
    stoplive();
    disconnectIpc();
    return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(ppsdevice) = {
    "ppsdevice",
    "vidsrc",
    module_init,
    module_close
};