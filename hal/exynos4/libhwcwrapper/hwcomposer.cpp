/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hwcomposer.h"

/*****************************************************************************/

static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static struct hw_module_methods_t hwc_module_methods = {
    open: hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
    common: {
        tag: HARDWARE_MODULE_TAG,
        version_major: 1,
        version_minor: 0,
        id: HWC_HARDWARE_MODULE_ID,
        name: "Exynos4 HWComposer Wrapper",
        author: "The NamelessRom Project",
        methods: &hwc_module_methods,
        .dso = NULL, /* remove compilation warnings */
        .reserved = {0}, /* remove compilation warnings */
    },
};

/******************************************************************************/

/* needed for wrapper */
static hwc_module_t *gVendorModule = NULL;

#define VENDOR_CALL(device, func, ...) ({ \
    struct hwc_context_t *__wrapper_dev = (struct hwc_context_t*) device; \
    __wrapper_dev->vendor->func(__wrapper_dev->vendor, ##__VA_ARGS__); \
})

static int check_vendor_module()
{
    int rv = 0;
    ALOGD("%s", __FUNCTION__);

    if (gVendorModule) {
        return 0;
    }

    rv = hw_get_module_by_class(HWC_HARDWARE_MODULE_ID, "vendor", (const hw_module_t **)&gVendorModule);
    if (rv) {
        ALOGE("failed to open vendor hwcomposer module");
    }
    return rv;
}

/*****************************************************************************/

static void dump_layer(hwc_layer_1_t const* l) {
    ALOGD("\ttype=%d, flags=%08x, handle=%p, tr=%02x, blend=%04x, {%d,%d,%d,%d}, {%d,%d,%d,%d}",
            l->compositionType, l->flags, l->handle, l->transform, l->blending,
            l->sourceCrop.left,
            l->sourceCrop.top,
            l->sourceCrop.right,
            l->sourceCrop.bottom,
            l->displayFrame.left,
            l->displayFrame.top,
            l->displayFrame.right,
            l->displayFrame.bottom);
}

static int hwc_prepare(hwc_composer_device_1_t *dev,
        size_t numDisplays, hwc_display_contents_1_t** displays)
{
    // spammy
    //ALOGD("%s", __FUNCTION__);

    return VENDOR_CALL(dev, prepare, numDisplays, displays);

    /*if (displays && (displays[0]->flags & HWC_GEOMETRY_CHANGED)) {
        for (size_t i=0 ; i<displays[0]->numHwLayers ; i++) {
            //dump_layer(&list->hwLayers[i]);
            displays[0]->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
        }
    }
    return 0;*/
}

static int hwc_set(hwc_composer_device_1_t *dev,
        size_t numDisplays, hwc_display_contents_1_t** displays)
{
    // spammy
    //ALOGD("%s", __FUNCTION__);

    return VENDOR_CALL(dev, set, numDisplays, displays);

    /*for (size_t i=0 ; i<list->numHwLayers ; i++) {
        dump_layer(&list->hwLayers[i]);
    }*/

    /*EGLBoolean sucess = eglSwapBuffers((EGLDisplay)displays[0]->dpy,
        (EGLSurface)displays[0]->sur);
    if (!sucess) {
        return HWC_EGL_ERROR;
    }
    return 0;*/
}

static int hwc_device_close(struct hw_device_t *dev)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;

    ALOGD("%s", __FUNCTION__);

    if ((ctx->vendor->common.close((hw_device_t* )(ctx->vendor)))) {
        ALOGE("vendor hwcomposer close fail");
    }

    if (ctx) {
        free(ctx);
    }
    return 0;
}

static int hwc_eventControl(struct hwc_composer_device_1* dev, int dpy,
        int event, int enabled)
{
    int val = 0, rc = 0;
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;

    switch (event) {
    case HWC_EVENT_VSYNC:
        ALOGD("%s HWC_EVENT_VSYNC enabled=%d", __FUNCTION__, enabled);
        return VENDOR_CALL(dev, eventControl, dpy, event, enabled);
    }
    return -EINVAL;
}

static int hwc_blank(struct hwc_composer_device_1 *dev, int dpy, int blank)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;

    ALOGD("%s blank=%d", __FUNCTION__, blank);

    return VENDOR_CALL(dev, blank, dpy, blank);
}

static int hwc_query(struct hwc_composer_device_1* dev,
        int what, int* value)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;

    ALOGD("%s", __FUNCTION__);

    switch (what) {
    case HWC_BACKGROUND_LAYER_SUPPORTED:
        // stock blob do support background layer
        value[0] = 1;
        break;
    case HWC_VSYNC_PERIOD:
        // vsync period in nanosecond
        value[0] = _VSYNC_PERIOD / ctx->gralloc->fps;
        break;
    default:
        // unsupported query
        return -EINVAL;
    }
    return 0;
}

static void hook_invalidate(const struct hwc_procs* procs)
{
    struct hwc_hooks_t* hooks = (struct hwc_hooks_t*)procs;

    ALOGD("%s", __FUNCTION__);

    if (hooks && hooks->procs && hooks->procs->invalidate) {
        hooks->procs->invalidate(hooks->procs);
    }
}

static void hook_vsync(const struct hwc_procs* procs, int disp, int64_t timestamp)
{
    struct hwc_hooks_t* hooks = (struct hwc_hooks_t*)procs;

    ALOGD("%s", __FUNCTION__);

    if (hooks && hooks->procs && hooks->procs->vsync) {
        hooks->procs->vsync(hooks->procs, disp, timestamp);
    }
}

static void hook_hotplug(const struct hwc_procs* procs, int disp, int connected)
{
    struct hwc_hooks_t* hooks = (struct hwc_hooks_t*)procs;

    ALOGD("%s", __FUNCTION__);

    if (hooks && hooks->procs && hooks->procs->hotplug) {
        hooks->procs->hotplug(hooks->procs, disp, connected);
    }
}

static void hwc_registerProcs(struct hwc_composer_device_1* dev,
        hwc_procs_t const* procs)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
    ctx->procs = const_cast<hwc_procs_t *>(procs);

    // Set up hooks and send them to vendor
    // ctx is already set to 0
    ctx->vendor_procs.hooks.invalidate = &hook_invalidate;
    ctx->vendor_procs.hooks.vsync = &hook_vsync;
    ctx->vendor_procs.hooks.hotplug = &hook_hotplug;

    ctx->vendor_procs.procs = ctx->procs;

    ALOGD("%s before VENDOR_CALL", __FUNCTION__);

    VENDOR_CALL(dev, registerProcs, (hwc_procs_t *) &ctx->vendor_procs.hooks);
}

static int hwc_getDisplayConfigs(struct hwc_composer_device_1* dev, int disp,
    uint32_t* configs, size_t* numConfigs)
{
    ALOGD("%s", __FUNCTION__);

    return VENDOR_CALL(dev, getDisplayConfigs, disp, configs, numConfigs);
}

static int hwc_getDisplayAttributes(struct hwc_composer_device_1* dev, int disp,
    __unused uint32_t config, const uint32_t* attributes, int32_t* values)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;
    int i = 0;

    ALOGD("%s", __FUNCTION__);

    while(attributes[i] != HWC_DISPLAY_NO_ATTRIBUTE) {
        switch(disp) {
        case 0:

            switch(attributes[i]) {
            case HWC_DISPLAY_VSYNC_PERIOD: /* The vsync period in nanoseconds */
                values[i] = ctx->vsync_period;
                break;

            case HWC_DISPLAY_WIDTH: /* The number of pixels in the horizontal and vertical directions. */
                values[i] = ctx->width;
                break;

            case HWC_DISPLAY_HEIGHT:
                values[i] = ctx->height;
                break;

            case HWC_DISPLAY_DPI_X:
                values[i] = ctx->xdpi;
                break;

            case HWC_DISPLAY_DPI_Y:
                values[i] = ctx->ydpi;
                break;

            default:
                ALOGE("%s::unknown display attribute %d", __func__, attributes[i]);
                return -EINVAL;
            }
            break;

        case 1:
            // TODO: no hdmi at the moment
            break;

        default:
            ALOGE("%s::unknown display %d", __func__, disp);
            return -EINVAL;
        }

        i++;
    }
    return 0;
}

/*****************************************************************************/

static int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
    int status = -EINVAL;
    struct hwc_context_t *dev = NULL;
    int refreshRate = 0;
    unsigned int i = 0;

    ALOGD("%s", __FUNCTION__);

    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {
        if (check_vendor_module()) {
            return -EINVAL;
        }

        dev = (hwc_context_t*)malloc(sizeof(*dev));

        /* initialize our state here */
        memset(dev, 0, sizeof(*dev));

        /* get gralloc module */
        if (hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const hw_module_t **) &dev->gralloc) != 0) {
            ALOGE("Fail on loading gralloc module");
            goto fail;
        }

        /* initialize the procs */
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = HWC_DEVICE_API_VERSION_1_1;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = hwc_device_close;

        dev->device.prepare = hwc_prepare;
        dev->device.set = hwc_set;
        dev->device.eventControl = hwc_eventControl;
        dev->device.blank = hwc_blank;
        dev->device.query = hwc_query;
        dev->device.registerProcs = hwc_registerProcs;
        //dev->device.dump
        dev->device.getDisplayConfigs = hwc_getDisplayConfigs;
        dev->device.getDisplayAttributes = hwc_getDisplayAttributes;

        *device = &dev->device.common;

        /* open all windows */
        for (i = 0; i < NUM_HW_WINDOWS; i++) {
            if (window_open(&(dev->win[i]), i) < 0) {
                ALOGE("%s:: Failed to open window %d device ", __func__, i);
                status = -EINVAL;
                goto err;
            }
        }

        /* query global LCD info */
        if (window_open(&dev->win_fb0, 2) < 0) {
            ALOGE("%s:: Failed to open window %d device ", __func__, 2);
            status = -EINVAL;
            goto err;
        }

        if (window_get_var_info(dev->win_fb0.fd, &dev->lcd_info) < 0) {
            ALOGE("%s::window_get_global_lcd_info is failed : %s",
                    __func__, strerror(errno));
            status = -EINVAL;
            goto err;
        }

        memcpy(&dev->win_fb0.lcd_info, &dev->lcd_info, sizeof(struct fb_var_screeninfo));
        memcpy(&dev->win_fb0.var_info, &dev->lcd_info, sizeof(struct fb_var_screeninfo));

        refreshRate = 1000000000000LLU /
            (
                uint64_t( dev->lcd_info.upper_margin + dev->lcd_info.lower_margin + dev->lcd_info.yres)
                * ( dev->lcd_info.left_margin  + dev->lcd_info.right_margin + dev->lcd_info.xres)
                * dev->lcd_info.pixclock
            );

        if (refreshRate == 0) {
            ALOGD("%s:: Invalid refresh rate, using 60 Hz", __func__);
            refreshRate = 60;  /* 60 Hz */
        }

        dev->vsync_period = _VSYNC_PERIOD / refreshRate;

        dev->width = dev->lcd_info.xres;
        dev->height = dev->lcd_info.yres;
        dev->xdpi = (dev->lcd_info.xres * 25.4f * 1000.0f) / dev->lcd_info.width;
        dev->ydpi = (dev->lcd_info.yres * 25.4f * 1000.0f) / dev->lcd_info.height;

        ALOGI("using\nxres         = %d px\nyres         = %d px\nwidth        = %d mm (%f dpi)\nheight       = %d mm (%f dpi)\nrefresh rate = %d Hz",
                dev->width, dev->height, dev->lcd_info.width, dev->xdpi / 1000.0f, dev->lcd_info.height, dev->ydpi / 1000.0f, refreshRate);

        dev->win_fb0.rect_info.x = 0;
        dev->win_fb0.rect_info.y = 0;
        dev->win_fb0.rect_info.w = dev->lcd_info.xres;
        dev->win_fb0.rect_info.h = dev->lcd_info.yres;

        if (window_get_info(&dev->win_fb0) < 0) {
            ALOGE("%s::window_get_info is failed : %s", __func__, strerror(errno));
            status = -EINVAL;
            goto err;
        }

        dev->win_fb0.size = dev->win_fb0.var_info.yres * dev->win_fb0.fix_info.line_length;
        dev->win_fb0.power_state = 1;

        dev->win_fb0.buf_index = 0;

        for(i = 0; i < NUM_OF_WIN_BUF; i++) {
            dev->win_fb0.addr[i] = dev->win_fb0.fix_info.smem_start + (dev->win_fb0.size * i);
            ALOGE("%s:: win-%d addr[%d] = 0x%x\n", __func__, 2, i, dev->win_fb0.addr[i]);
        }

        if (DEBUG) {
            dump_win(&dev->win_fb0);
        }

        /* hwc blob is going to open fb windows, so let's close them atm in order to be able
           to call open function in hwc blob */
        if (window_close(&dev->win_fb0) < 0) {
            ALOGE("%s::window_close() fail", __func__);
        }

        for (i = 0; i < NUM_HW_WINDOWS; i++) {
            if (window_close(&dev->win[i]) < 0) {
                ALOGE("%s::window_close() fail", __func__);
            }
        }

        // now we can safe call to hwc blob open()
        if ((gVendorModule->common.methods->open((const hw_module_t*)gVendorModule, name, (hw_device_t**)&(dev->vendor)))) {
            ALOGE("vendor hwcomposer open fail");
            goto fail;
        }
        ALOGD("%s: got vendor hwcomposer device 0x%08X", __FUNCTION__, (void *)(dev->vendor));

        status = 0;
    }
    return status;

err:
    if (window_close(&dev->win_fb0) < 0) {
        ALOGE("%s::window_close() fail", __func__);
    }

    for (i = 0; i < NUM_HW_WINDOWS; i++) {
        if (window_close(&dev->win[i]) < 0) {
            ALOGE("%s::window_close() fail", __func__);
        }
    }

fail:
    if (dev) {
        free(dev);
    }

    return -EINVAL;
}
