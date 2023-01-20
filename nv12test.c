//
// Minimal gbm client to see if libgbm can work with NV12.
// Bram Stolk (bram.stolk@canonical.com)
//
// $ cc -Wall -Wextra -g nv12test.c -o nv12test -lgbm
// $ ./nv12test /dev/dri/card0
//

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <gbm.h>
#include <drm/drm_fourcc.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#define IMW	1280
#define IMH	720

#define ENTRY(A) \
	{ A, ##A }


// Taken from /usr/include/gbm.h
static const uint32_t candidates[] =
{
GBM_FORMAT_C8,
GBM_FORMAT_R8,
GBM_FORMAT_R16,
GBM_FORMAT_GR88,
GBM_FORMAT_RG1616,
GBM_FORMAT_GR1616,
GBM_FORMAT_RGB332,
GBM_FORMAT_BGR233,
GBM_FORMAT_XRGB4444,
GBM_FORMAT_XBGR4444,
GBM_FORMAT_RGBX4444,
GBM_FORMAT_BGRX4444,
GBM_FORMAT_ARGB4444,
GBM_FORMAT_ABGR4444,
GBM_FORMAT_RGBA4444,
GBM_FORMAT_BGRA4444,
GBM_FORMAT_XRGB1555,
GBM_FORMAT_XBGR1555,
GBM_FORMAT_RGBX5551,
GBM_FORMAT_BGRX5551,
GBM_FORMAT_ARGB1555,
GBM_FORMAT_ABGR1555,
GBM_FORMAT_RGBA5551,
GBM_FORMAT_BGRA5551,
GBM_FORMAT_RGB565,
GBM_FORMAT_BGR565,
GBM_FORMAT_RGB888,
GBM_FORMAT_BGR888,
GBM_FORMAT_XRGB8888,
GBM_FORMAT_XBGR8888,
GBM_FORMAT_RGBX8888,
GBM_FORMAT_BGRX8888,
GBM_FORMAT_ARGB8888,
GBM_FORMAT_ABGR8888,
GBM_FORMAT_RGBA8888,
GBM_FORMAT_BGRA8888,
GBM_FORMAT_XRGB2101010,
GBM_FORMAT_XBGR2101010,
GBM_FORMAT_RGBX1010102,
GBM_FORMAT_BGRX1010102,
GBM_FORMAT_ARGB2101010,
GBM_FORMAT_ABGR2101010,
GBM_FORMAT_RGBA1010102,
GBM_FORMAT_BGRA1010102,
GBM_FORMAT_XBGR16161616,
GBM_FORMAT_ABGR16161616,
GBM_FORMAT_XBGR16161616F,
GBM_FORMAT_ABGR16161616F,
GBM_FORMAT_YUYV,
GBM_FORMAT_YVYU,
GBM_FORMAT_UYVY,
GBM_FORMAT_VYUY,
GBM_FORMAT_AYUV,
GBM_FORMAT_NV12,
GBM_FORMAT_NV21,
GBM_FORMAT_NV16,
GBM_FORMAT_NV61,
GBM_FORMAT_YUV410,
GBM_FORMAT_YVU410,
GBM_FORMAT_YUV411,
GBM_FORMAT_YVU411,
GBM_FORMAT_YUV420,
GBM_FORMAT_YVU420,
GBM_FORMAT_YUV422,
GBM_FORMAT_YVU422,
GBM_FORMAT_YUV444,
GBM_FORMAT_YVU444,
	0
};


uint8_t* make_checkerboard_nv12(int w, int h, size_t* sz)
{
	const size_t sz_Y = w*h;
	const size_t sz_U = w*h/4;
	const size_t sz_V = w*h/4;
	*sz = sz_Y + sz_U + sz_V;
	uint8_t* buf = (uint8_t*)malloc(*sz);
	assert(buf);
	uint8_t* writer = buf;
	// Write Y checkerboard of 8x8 tiles:
	for (int y=0; y<h; ++y)
		for (int x=0; x<w; ++x)
		{
			int yy=y>>3;
			int xx=x>>3;
			*writer++ = ((yy+xx)&1) ? 0xff : 0x80;
		}
	// Write UV checkerboard at half-res.
	for (int y=0; y<h/2; ++y)
		for (int x=0; x<w/2; ++x)
		{
			*writer++ = x<w/2 ? 0x40 : 0xc0;
			*writer++ = y<h/2 ? 0x70 : 0xe0;
		}
	const size_t written = (size_t) (writer - buf);
	assert(written == *sz);
	return buf;
}


int main(int argc, char* argv[])
{
	if (argc != 2 && argc != 1)
	{
		fprintf(stderr, "Usage: %s [/dev/dri/card0]\n", argv[0]);
		exit(1);
	}
	const char* dname = argc == 2 ? argv[1] : "/dev/dri/card0";

	int fd = open(dname, O_RDWR);
	assert(fd > 0);

	struct gbm_device* gbm = gbm_create_device(fd);
	assert(gbm);

	const char* bename = gbm_device_get_backend_name(gbm);
	fprintf(stderr, "Using backend %s\n", bename);

	const uint32_t usage = 
		0
//		| GBM_BO_USE_RENDERING
		| GBM_BO_USE_WRITE
		| GBM_BO_USE_LINEAR
		| GBM_BO_USE_SCANOUT
		;

	for (int i=0; candidates[i]; ++i)
	{
		const uint32_t cand = candidates[i];
		const int supp = gbm_device_is_format_supported(gbm, cand, usage);
		char nm[5] = {0,};
		*((uint32_t*)nm) = cand;
		fprintf(stderr, "%4s: %s\n", nm, supp ? "[SUPPORTED]" : "");
	}

	size_t bufsz=0;
	const uint8_t* buf = make_checkerboard_nv12(IMW, IMH, &bufsz);

	const uint32_t fmt = GBM_FORMAT_NV12;
	struct gbm_bo* bo = gbm_bo_create(gbm, IMW, IMH, fmt, usage);
	if (!bo)
	{
		fprintf(stderr, "Failed to create buffer object.\n");
		exit(2);
	}

	const uint32_t w = gbm_bo_get_width(bo);
	const uint32_t h = gbm_bo_get_height(bo);
	const int planecount = gbm_bo_get_plane_count(bo);
	const uint32_t bpp = gbm_bo_get_bpp(bo);
	fprintf(stderr, "Created GBM buffer object of format NV12 and dimension %dx%d\n", w,h);
	fprintf(stderr, "Bpp: %d\n", bpp);
	for (int p=0; p<planecount; ++p)
	{
		const uint32_t stride = gbm_bo_get_stride_for_plane(bo, 0);
		fprintf(stderr, "Stride for plane %d: %d\n", p, stride);
	}

#if 0
	// We can only call bo_write for a buffer object that has usage RENDERING.
	gbm_bo_write(bo, buf, w /*bufsz*/);
#endif

	uint32_t stride = 0;
	void* mapping = 0;
	uint8_t* m = gbm_bo_map
	(
		bo,
		0,0,
		w,h,
		GBM_BO_TRANSFER_WRITE,
		&stride,
		&mapping
	);

	fprintf(stderr, "m = %p\n", m);
	fprintf(stderr, "mapping = %p\n", mapping);

	memcpy(m, buf, bufsz);

	gbm_bo_unmap(bo, mapping);

	union gbm_bo_handle handle = gbm_bo_get_handle(bo);
	if (handle.s32 < 0)
	{
		fprintf(stderr, "gbm_bo_get_handle failed\n");
	}

	// It would be nice if we could now display the buffer somehow.
	// We could attach it to a framebuffer:

	const char* s = getenv("XDG_SESSION_TYPE");
	const int on_tty = (!strcmp(s, "tty"));

	if (on_tty)
	{

		if (drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1) != 0)
		{
		fprintf(stderr, "DRM_CLIENT_CAP_ATOMIC could not be set: %s\n", strerror(errno));
		}
		drmModeAtomicReq* req = drmModeAtomicAlloc();

		uint32_t handles[4] =
		{
			handle.s32,
			handle.s32,
			0,
			0,
		};
		uint32_t strides[4] =
		{
			gbm_bo_get_stride(bo),
			gbm_bo_get_stride(bo),
			0,
			0,
		};
		uint32_t offsets[4] = { 0,0,0,0, };
		uint32_t fb_id=0;
		int add_rv = drmModeAddFB2(fd, IMW, IMH, DRM_FORMAT_NV12, handles, strides, offsets, &fb_id, 0);
		if (add_rv < 0)
		{
			fprintf(stderr, "drmModeAddFB2() failed: %s\n", strerror(errno));
		}

		const uint32_t commit_flags = DRM_MODE_ATOMIC_ALLOW_MODESET;
		int commit_rv = drmModeAtomicCommit(fd, req, commit_flags, NULL);
		if (commit_rv != 0)
		{
			fprintf(stderr, "drmModeAtomicCommit failed: %s\n", strerror(errno));
		}

		int crtc_id = 80;
		int conn_id = 236;
		drmModeModeInfo* mode_inf = 0;
		int count=0;
		int setcrtc_rv = drmModeSetCrtc
		(
			fd,
			crtc_id,
			fb_id,
			0,0,
			&conn_id,
			count,
			mode_inf
		);
		if (setcrtc_rv != 0)
		{
			fprintf(stderr, "drmModeSetCrtc failed: %s\n", strerror(errno));
		}

		sleep(8);

		drmModeAtomicFree(req);
	}

	// Clean up

	gbm_bo_destroy(bo);

	close(fd);

	return 0;
}

