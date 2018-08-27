
#ifndef ANDROID_HWCOMPOSER_H
#define ANDROID_HWCOMPOSER_H

#include <hardware/hardware.h>

#include <fcntl.h>
#include <errno.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hwcomposer.h>

#include <EGL/egl.h>

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sync/sync.h>
#include <malloc.h>

#include "gralloc_priv.h"
#include "s3c_lcd.h"
#include "secion.h"

#include "window.h"

#ifndef _VSYNC_PERIOD
#define _VSYNC_PERIOD 1000000000UL
#endif

const size_t NUM_HW_WINDOWS = 5;
const size_t NO_FB_NEEDED = NUM_HW_WINDOWS + 1;
const size_t NUM_OF_WIN_BUF = 3;

#define DEBUG 1

#ifdef ALOGD
#undef ALOGD
#endif
#define ALOGD(...) if (DEBUG) ((void)ALOG(LOG_DEBUG, LOG_TAG, __VA_ARGS__))

#define EXYNOS4_ALIGN( value, base ) (((value) + ((base) - 1)) & ~((base) - 1))

struct hwc_hooks_t {
    hwc_procs_t  hooks;
    hwc_procs_t *procs;
};

struct sec_rect {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;
};

struct ion_hdl{
    ion_client          client;
    struct secion_param param;
    ion_client          client2;
};

struct hwc_win_info_t {
    int        fd;
    int        size;
    sec_rect   rect_info;
    uint32_t   addr[NUM_OF_WIN_BUF];
    int        buf_index;

    int        power_state;
    int        blending;
    int        layer_index;
    int        status;
    int        vsync;

    struct fb_fix_screeninfo fix_info;
    struct fb_var_screeninfo var_info;
    struct fb_var_screeninfo lcd_info;

    struct secion_param secion_param[NUM_OF_WIN_BUF];
    int                 fence[NUM_OF_WIN_BUF];
};

struct hwc_context_t {
    hwc_composer_device_1_t   device;
    /* our private state goes below here */
    hwc_procs_t              *procs;

    struct hwc_composer_device_1 *vendor;
    struct hwc_hooks_t            vendor_procs;

    struct private_module_t  *gralloc;

    struct hwc_win_info_t     win[NUM_HW_WINDOWS];
    struct hwc_win_info_t     win_fb0;
    struct hwc_win_info_t     win_fbdev1;
    struct fb_var_screeninfo  lcd_info;

    struct ion_hdl            ion_hdl;

    int    width;
    int    height;
    int    xres;
    int    yres;
    float  xdpi;
    float  ydpi;
    int    vsync_period;

};

#endif //ANDROID_HWCOMPOSER_H
