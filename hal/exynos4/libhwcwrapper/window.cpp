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

void dump_win(struct hwc_win_info_t *win)
{
    unsigned int i = 0;

    ALOGD("Dump Window Information");
    ALOGD("win->fd = %d", win->fd);
    ALOGD("win->size = %d", win->size);
    ALOGD("win->rect_info.x = %d", win->rect_info.x);
    ALOGD("win->rect_info.y = %d", win->rect_info.y);
    ALOGD("win->rect_info.w = %d", win->rect_info.w);
    ALOGD("win->rect_info.h = %d", win->rect_info.h);

    for (i = 0; i < NUM_OF_WIN_BUF; i++) {
        ALOGD("win->addr[%d] = 0x%x", i, win->addr[i]);
    }

    ALOGD("win->buf_index = %d", win->buf_index);
    ALOGD("win->power_state = %d", win->power_state);
    ALOGD("win->blending = %d", win->blending);
    ALOGD("win->layer_index = %d", win->layer_index);
    ALOGD("win->status = %d", win->status);
    ALOGD("win->vsync = %d", win->vsync);
    ALOGD("win->fix_info.smem_start = 0x%x", win->fix_info.smem_start);
    ALOGD("win->fix_info.line_length = %d", win->fix_info.line_length);
    ALOGD("win->var_info.xres = %d", win->var_info.xres);
    ALOGD("win->var_info.yres = %d", win->var_info.yres);
    ALOGD("win->var_info.xres_virtual = %d", win->var_info.xres_virtual);
    ALOGD("win->var_info.yres_virtual = %d", win->var_info.yres_virtual);
    ALOGD("win->var_info.xoffset = %d", win->var_info.xoffset);
    ALOGD("win->var_info.yoffset = %d", win->var_info.yoffset);
    ALOGD("win->lcd_info.xres = %d", win->lcd_info.xres);
    ALOGD("win->lcd_info.yres = %d", win->lcd_info.yres);
    ALOGD("win->lcd_info.xoffset = %d", win->lcd_info.xoffset);
    ALOGD("win->lcd_info.yoffset = %d", win->lcd_info.yoffset);
    ALOGD("win->lcd_info.bits_per_pixel = %d", win->lcd_info.bits_per_pixel);
}

int window_open(struct hwc_win_info_t *win, int id)
{
    int fd = 0;
    char name[64];
    int vsync = 1;
    int real_id;

    char const * const device_template = "/dev/graphics/fb%u";
    // window & FB maping
    // fb0 -> win-id : 2
    // fb1 -> win-id : 3
    // fb2 -> win-id : 4
    // fb3 -> win-id : 0
    // fb4 -> win_id : 1
    // it is pre assumed that ...win0 or win1 is used here..

    if (id <= NUM_HW_WINDOWS - 1) {
        real_id = (id + 3) % NUM_HW_WINDOWS;
    } else if (id == NUM_HW_WINDOWS) {
        real_id = id;
    } else {
        ALOGE("%s::id(%d) is weird", __func__, id);
        goto error;
    }

    snprintf(name, 64, device_template, real_id);

    win->fd = open(name, O_RDWR);
    if (win->fd <= 0) {
        ALOGE("%s::Failed to open window device (%s) : %s",
                __func__, strerror(errno), name);
        goto error;
    }

    return 0;

error:
    if (0 < win->fd)
        close(win->fd);
    win->fd = 0;

    return -1;
}

int window_close(struct hwc_win_info_t *win)
{
    int ret = 0;

    if (0 < win->fd) {
        ret = close(win->fd);
    }
    win->fd = 0;

    return ret;
}

int window_get_info(struct hwc_win_info_t *win)
{
    if (ioctl(win->fd, FBIOGET_FSCREENINFO, &win->fix_info) < 0) {
        ALOGE("FBIOGET_FSCREENINFO failed : %s", strerror(errno));

        dump_win(win);
        win->fix_info.smem_start = NULL;
        return -1;
    }

    return 0;
}

int window_get_var_info(int fd, struct fb_var_screeninfo *var_info)
{
    if (ioctl(fd, FBIOGET_VSCREENINFO, var_info) < 0) {
         ALOGE("FBIOGET_VSCREENINFO failed : %s", strerror(errno));
         return -1;
     }

     return 0;
}
