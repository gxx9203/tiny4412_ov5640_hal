#ifndef ANDROID_WINDOW_H
#define ANDROID_WINDOW_H

void dump_win(struct hwc_win_info_t *win);

int window_open(struct hwc_win_info_t *win, int id);
int window_close(struct hwc_win_info_t *win);

int window_get_info(struct hwc_win_info_t *win);
int window_get_var_info(int fd, struct fb_var_screeninfo *var_info);

#endif //ANDROID_WINDOW_H
