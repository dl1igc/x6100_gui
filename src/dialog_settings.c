/*
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 *
 *  Xiegu X6100 LVGL GUI
 *
 *  Copyright (c) 2022-2023 Belousov Oleg aka R1CBU
 */

#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>

#include "lvgl/lvgl.h"
#include "dialog.h"
#include "dialog_settings.h"
#include "styles.h"
#include "params.h"
#include "backlight.h"

static lv_obj_t     *dialog;
static lv_obj_t     *grid;

#define SMALL_WIDTH     115
#define SMALL_PAD       5

static lv_coord_t   col_dsc[] = { 740 - (SMALL_WIDTH + SMALL_PAD) * 3, SMALL_WIDTH, SMALL_WIDTH, SMALL_WIDTH, LV_GRID_TEMPLATE_LAST };
static lv_coord_t   row_dsc[] = { 54, 54, 54, 54, 54, 54, 54, 54, LV_GRID_TEMPLATE_LAST };

static time_t       now;
struct tm           ts;

static lv_obj_t     *day;
static lv_obj_t     *month;
static lv_obj_t     *year;
static lv_obj_t     *hour;
static lv_obj_t     *min;
static lv_obj_t     *sec;

/* Datetime */

static void datetime_update_cb(lv_event_t * e) {
    ts.tm_mday = lv_spinbox_get_value(day);
    ts.tm_mon = lv_spinbox_get_value(month) - 1;
    ts.tm_year = lv_spinbox_get_value(year) - 1900;
    ts.tm_hour = lv_spinbox_get_value(hour);
    ts.tm_min = lv_spinbox_get_value(min);
    ts.tm_sec = lv_spinbox_get_value(sec);

    /* Set system */
    
    struct timespec tp;
    
    tp.tv_sec = mktime(&ts);
    tp.tv_nsec = 0;

    clock_settime(CLOCK_REALTIME, &tp);
    
    /* Set RTC */
    
    int rtc = open("/dev/rtc1", O_WRONLY);
    
    if (rtc > 0) {
        ioctl(rtc, RTC_SET_TIME, &ts);
        close(rtc);
    } else {
        LV_LOG_ERROR("Set RTC");
    }
}

static uint8_t make_date(uint8_t row) {
    lv_obj_t    *obj;
    uint8_t     col = 0;

    /* Label */

    obj = lv_label_create(grid);

    lv_label_set_text(obj, "Day, Month, Year");
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col++, 1, LV_GRID_ALIGN_CENTER, row, 1);

    /* Day */

    obj = lv_spinbox_create(grid);
    day = obj;

    dialog_item(obj);

    lv_spinbox_set_value(obj, ts.tm_mday);
    lv_spinbox_set_range(obj, 1, 31);
    lv_spinbox_set_digit_format(obj, 2, 0);
    lv_spinbox_set_digit_step_direction(obj, LV_DIR_LEFT);
    lv_obj_set_size(obj, SMALL_WIDTH, 56);
    
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col++, 1, LV_GRID_ALIGN_CENTER, row, 1);
    lv_obj_add_event_cb(obj, datetime_update_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* Month */

    obj = lv_spinbox_create(grid);
    month = obj;

    dialog_item(obj);

    lv_spinbox_set_value(obj, ts.tm_mon + 1);
    lv_spinbox_set_range(obj, 1, 12);
    lv_spinbox_set_digit_format(obj, 2, 0);
    lv_spinbox_set_digit_step_direction(obj, LV_DIR_LEFT);
    lv_obj_set_size(obj, SMALL_WIDTH, 56);
    
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col++, 1, LV_GRID_ALIGN_CENTER, row, 1);
    lv_obj_add_event_cb(obj, datetime_update_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* Year */

    obj = lv_spinbox_create(grid);
    year = obj;

    dialog_item(obj);

    lv_spinbox_set_value(obj, ts.tm_year + 1900);
    lv_spinbox_set_range(obj, 2020, 2038);
    lv_spinbox_set_digit_format(obj, 4, 0);
    lv_spinbox_set_digit_step_direction(obj, LV_DIR_LEFT);
    lv_obj_set_size(obj, SMALL_WIDTH, 56);
    
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col++, 1, LV_GRID_ALIGN_CENTER, row, 1);
    lv_obj_add_event_cb(obj, datetime_update_cb, LV_EVENT_VALUE_CHANGED, NULL);

    return row + 1;
}

static uint8_t make_time(uint8_t row) {
    lv_obj_t    *obj;
    uint8_t     col = 0;

    /* Label */

    obj = lv_label_create(grid);

    lv_label_set_text(obj, "Hour, Min, Sec");
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col++, 1, LV_GRID_ALIGN_CENTER, row, 1);

    /* Hour */

    obj = lv_spinbox_create(grid);
    hour = obj;

    dialog_item(obj);

    lv_spinbox_set_value(obj, ts.tm_hour);
    lv_spinbox_set_range(obj, 0, 23);
    lv_spinbox_set_digit_format(obj, 2, 0);
    lv_spinbox_set_digit_step_direction(obj, LV_DIR_LEFT);
    lv_obj_set_size(obj, SMALL_WIDTH, 56);
    
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col++, 1, LV_GRID_ALIGN_CENTER, row, 1);
    lv_obj_add_event_cb(obj, datetime_update_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* Min */

    obj = lv_spinbox_create(grid);
    min = obj;

    dialog_item(obj);

    lv_spinbox_set_value(obj, ts.tm_min);
    lv_spinbox_set_range(obj, 0, 59);
    lv_spinbox_set_digit_format(obj, 2, 0);
    lv_spinbox_set_digit_step_direction(obj, LV_DIR_LEFT);
    lv_obj_set_size(obj, SMALL_WIDTH, 56);
    
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col++, 1, LV_GRID_ALIGN_CENTER, row, 1);
    lv_obj_add_event_cb(obj, datetime_update_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* Sec */

    obj = lv_spinbox_create(grid);
    sec = obj;

    dialog_item(obj);

    lv_spinbox_set_value(obj, ts.tm_sec);
    lv_spinbox_set_range(obj, 0, 59);
    lv_spinbox_set_digit_format(obj, 2, 0);
    lv_spinbox_set_digit_step_direction(obj, LV_DIR_LEFT);
    lv_obj_set_size(obj, SMALL_WIDTH, 56);
    
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col++, 1, LV_GRID_ALIGN_CENTER, row, 1);
    lv_obj_add_event_cb(obj, datetime_update_cb, LV_EVENT_VALUE_CHANGED, NULL);

    return row + 1;
}

/* Backlight */

static void backlight_timeout_update_cb(lv_event_t * e) {
    lv_obj_t *obj = lv_event_get_target(e);

    params_lock();
    params.brightness_timeout = lv_spinbox_get_value(obj);
    params_unlock(&params.durty.brightness_timeout);

    backlight_tick();
}

static void backlight_brightness_update_cb(lv_event_t * e) {
    lv_obj_t *obj = lv_event_get_target(e);

    params_lock();
    params.brightness_normal = lv_slider_get_value(obj);
    params_unlock(&params.durty.brightness_normal);

    params_lock();
    params.brightness_idle = lv_slider_get_left_value(obj);
    params_unlock(&params.durty.brightness_idle);

    backlight_set_brightness(params.brightness_normal);
}

static uint8_t make_backlight(uint8_t row) {
    lv_obj_t    *obj;
    uint8_t     col = 0;

    /* Label */

    obj = lv_label_create(grid);

    lv_label_set_text(obj, "Timeout, Brightness");
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col++, 1, LV_GRID_ALIGN_CENTER, row, 1);
    
    /* Timeout */

    obj = lv_spinbox_create(grid);

    dialog_item(obj);

    lv_spinbox_set_value(obj, params.brightness_timeout);
    lv_spinbox_set_range(obj, 5, 120);
    lv_spinbox_set_digit_format(obj, 3, 0);
    lv_spinbox_set_digit_step_direction(obj, LV_DIR_LEFT);
    lv_obj_set_size(obj, SMALL_WIDTH, 56);
    
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col++, 1, LV_GRID_ALIGN_CENTER, row, 1);
    lv_obj_add_event_cb(obj, backlight_timeout_update_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* Brightness */

    obj = lv_obj_create(grid);
    
    lv_obj_set_size(obj, SMALL_WIDTH * 2 + SMALL_PAD, 56);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_START, col++, 2, LV_GRID_ALIGN_CENTER, row, 1);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(obj);

    obj = lv_slider_create(obj);

    dialog_item(obj);
    
    lv_slider_set_mode(obj, LV_SLIDER_MODE_RANGE);
    lv_slider_set_value(obj, params.brightness_normal, LV_ANIM_OFF);
    lv_slider_set_left_value(obj, params.brightness_idle, LV_ANIM_OFF);
    lv_slider_set_range(obj, -1, 9);
    lv_obj_set_width(obj, SMALL_WIDTH * 2 - 30);
    lv_obj_center(obj);

    lv_obj_add_event_cb(obj, backlight_brightness_update_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

lv_obj_t * dialog_settings(lv_obj_t *parent) {
    dialog = dialog_init(parent);
    grid = lv_obj_create(dialog);
    
    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
    lv_obj_set_size(grid, 780, 330);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);
    
    lv_obj_set_style_text_color(grid, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(grid, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(grid, SMALL_PAD, 0);
    lv_obj_set_style_pad_row(grid, 5, 0);
    
    lv_obj_center(grid);

    uint8_t row = 0;

    now = time(NULL);
    struct tm *t = localtime(&now);

    memcpy(&ts, t, sizeof(ts));
    
    row = make_date(row);
    row = make_time(row);
    row = make_backlight(row);

    return dialog;
}