#include "PPCS_API.h"
#include "PPCS_Error.h"  
#include "PPCS_Type.h"
#include <re.h>
#include <rem.h>
#include <baresip.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

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

struct vidsrc_st {
	const struct vidsrc *vs;  /* inheritance */
	pthread_t thread;
	bool run;
	struct vidsz sz;
	int sindex;
	vidsrc_frame_h *frameh;
	void *arg;
};

static struct vidsrc *mod_avf;

static void destructor(void *arg)
{
	struct vidsrc_st *st = arg;

	if (st->run) {
		st->run = false;
		pthread_join(st->thread, NULL);
	}
}

static void *read_thread(void *data)
{
	return NULL;
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
	err = pthread_create(&st->thread, NULL, read_thread, st);
	if (err) {
		st->run = false;
		goto out;
	}

 out:
	if (err)
		mem_deref(st);
	else
		*stp = st;

	return err;
}

static int module_init(void)
{

	return vidsrc_register(&mod_avf, baresip_vidsrcl(),
			       "ppsdevice", alloc, NULL);
}


static int module_close(void)
{
	mod_avf = mem_deref(mod_avf);
	return 0;
}


EXPORT_SYM const struct mod_export DECL_EXPORTS(ppsdevice) = {
	"ppsdevice",
	"vidsrc",
	module_init,
	module_close
};