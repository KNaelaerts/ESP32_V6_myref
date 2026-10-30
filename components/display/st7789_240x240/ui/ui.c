#include "ui.h"

static void counter_cb(lv_timer_t *t)
{
    static int count;
    lv_label_set_text_fmt((lv_obj_t *)lv_timer_get_user_data(t), "%d", ++count);
}

void ui_init(void)
{
    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "ST7789\n240x240");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *counter = lv_label_create(scr);
    lv_label_set_text(counter, "0");
    lv_obj_set_style_text_color(counter, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_align(counter, LV_ALIGN_CENTER, 0, 20);
    lv_timer_create(counter_cb, 1000, counter);
}

void ui_tick(void)
{
}
