// main/demo_life_rpg.c —— AI Passport Life RPG 的 Phase 1 外壳。
//
// 这一阶段只验证三件事:
//   1. 能从官方主菜单进入 Life RPG 页面;
//   2. 240x320 屏幕上能显示 LIFE RPG / LV.01 / PRESS BUTTON;
//   3. OK 长按能回到官方菜单,页面退出时没有遗留 timer/task。
//
// 后续 Player / Quest / Pet / Storage 模块会从本文件拆出,不把业务逻辑堆在这里。

#include "demo.h"
#include "bsp_battery.h"
#include "ui_pixel.h"
#include "lvgl.h"

static lv_obj_t *s_scr;
static lv_obj_t *s_battery;
static lv_obj_t *s_mascot;

// 右上角显示电量。官方基线约定 UI 默认显示电池,读不到时优雅降级为 "--"。
static void refresh_battery(void)
{
    if (!s_battery) return;

    int soc = bsp_battery_soc();
    if (soc < 0) {
        lv_label_set_text(s_battery, "--%");
        lv_obj_set_style_text_color(s_battery, lv_color_hex(UI_MUTED), 0);
        return;
    }

    lv_label_set_text_fmt(s_battery, "%d%%", soc);
    lv_obj_set_style_text_color(s_battery,
                                (soc < 20) ? lv_color_hex(0xFF5A5A)
                                           : lv_color_hex(UI_INK),
                                0);
}

void demo_life_rpg_enter(void)
{
    s_scr = ui_pixel_screen_create("LIFE RPG");

    // 电量放在蓝天区域,避开右上角已有的白云装饰。
    s_battery = lv_label_create(s_scr);
    lv_obj_set_style_text_font(s_battery, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(s_battery, 196, 27);
    refresh_battery();

    lv_obj_t *panel = ui_pixel_panel_create(s_scr, 30, 86, 180, 120, UI_PAPER);

    lv_obj_t *level = lv_label_create(panel);
    lv_obj_set_style_text_font(level, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(level, lv_color_hex(UI_INK), 0);
    lv_label_set_text(level, "LV.01");
    lv_obj_align(level, LV_ALIGN_TOP_MID, 0, 24);

    lv_obj_t *hint = lv_label_create(panel);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(UI_SKY_DARK), 0);
    lv_label_set_text(hint, "PRESS BUTTON");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -18);

    s_mascot = ui_pixel_mascot_create(s_scr, 101, 238);
    lv_screen_load(s_scr);
}

void demo_life_rpg_exit(void)
{
    if (s_scr) {
        lv_obj_delete(s_scr);
        s_scr = NULL;
        s_battery = NULL;
        s_mascot = NULL;
    }
}

void demo_life_rpg_key(bsp_btn_t btn, bsp_btn_ev_t ev)
{
    (void)btn;

    // Phase 1 只做即时视觉反馈;进入/退出由 main.c 统一处理。
    if (ev == BSP_BTN_PRESS) {
        ui_pixel_mascot_jump(s_mascot);
    }
}
