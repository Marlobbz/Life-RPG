// main/demo_life_rpg.c —— AI Passport Life RPG 页面入口。
//
// Phase 2 阶段负责接入 Player / XP / Level / Stats 和本地持久化。
// 页面只做 UI 和按键分发;纯逻辑在 rpg_player.c,存储在 rpg_storage.c。
//
// 为了在没有 Quest 系统的阶段验证升级和保存,当前 OK 单击临时 +20 XP。
// 该调试行为会在 Phase 3 由 Quest 完成奖励替换。

#include "demo.h"
#include "bsp_battery.h"
#include "rpg_player.h"
#include "rpg_storage.h"
#include "ui_pixel.h"
#include "esp_log.h"
#include "lvgl.h"

static const char *TAG = "life_rpg";

#define RPG_PHASE2_DEBUG_XP 20u

static lv_obj_t *s_scr;
static lv_obj_t *s_battery;
static lv_obj_t *s_level;
static lv_obj_t *s_xp;
static lv_obj_t *s_stats;
static lv_obj_t *s_mascot;
static rpg_player_t s_player;

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

static void refresh_player(void)
{
    if (!s_level || !s_xp || !s_stats) return;

    lv_label_set_text_fmt(s_level, "LV.%02u", (unsigned)s_player.level);
    lv_label_set_text_fmt(s_xp, "XP %u / %u",
                          (unsigned)s_player.xp,
                          (unsigned)rpg_xp_to_next_level(s_player.level));
    lv_label_set_text_fmt(s_stats, "INT %u  STR %u\nAGI %u  WIS %u",
                          (unsigned)s_player.stats[RPG_STAT_INT],
                          (unsigned)s_player.stats[RPG_STAT_STR],
                          (unsigned)s_player.stats[RPG_STAT_AGI],
                          (unsigned)s_player.stats[RPG_STAT_WIS]);
}

void demo_life_rpg_enter(void)
{
    rpg_player_init(&s_player);

    esp_err_t storage_err = rpg_storage_init();
    if (storage_err != ESP_OK) {
        ESP_LOGW(TAG, "storage init failed, using default player: %s",
                 esp_err_to_name(storage_err));
    } else {
        storage_err = rpg_storage_load_player(&s_player);
        if (storage_err != ESP_OK) {
            ESP_LOGW(TAG, "player load failed, using default player: %s",
                     esp_err_to_name(storage_err));
        }
    }

    s_scr = ui_pixel_screen_create("LIFE RPG");

    // 电量放在蓝天区域,避开右上角已有的白云装饰。
    s_battery = lv_label_create(s_scr);
    lv_obj_set_style_text_font(s_battery, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(s_battery, 196, 27);
    refresh_battery();

    lv_obj_t *panel = ui_pixel_panel_create(s_scr, 20, 78, 200, 145, UI_PAPER);

    s_level = lv_label_create(panel);
    lv_obj_set_style_text_font(s_level, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(s_level, lv_color_hex(UI_INK), 0);
    lv_obj_align(s_level, LV_ALIGN_TOP_MID, 0, 8);

    s_xp = lv_label_create(panel);
    lv_obj_set_style_text_font(s_xp, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_xp, lv_color_hex(UI_SKY_DARK), 0);
    lv_obj_align(s_xp, LV_ALIGN_TOP_MID, 0, 38);

    s_stats = lv_label_create(panel);
    lv_obj_set_style_text_font(s_stats, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_stats, lv_color_hex(UI_INK), 0);
    lv_obj_align(s_stats, LV_ALIGN_TOP_MID, 0, 64);

    lv_obj_t *hint = lv_label_create(panel);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(UI_SKY_DARK), 0);
    lv_label_set_text(hint, "OK: +20 XP");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -8);

    s_mascot = ui_pixel_mascot_create(s_scr, 101, 238);
    refresh_player();
    lv_screen_load(s_scr);
}

void demo_life_rpg_exit(void)
{
    rpg_storage_save_player(&s_player);

    if (s_scr) {
        lv_obj_delete(s_scr);
        s_scr = NULL;
        s_battery = NULL;
        s_level = NULL;
        s_xp = NULL;
        s_stats = NULL;
        s_mascot = NULL;
    }
}

void demo_life_rpg_key(bsp_btn_t btn, bsp_btn_ev_t ev)
{
    if (btn == BSP_BTN_OK && ev == BSP_BTN_CLICK) {
        uint32_t levels_gained = rpg_player_add_xp(&s_player, RPG_PHASE2_DEBUG_XP);
        if (levels_gained > 0) {
            rpg_storage_save_player(&s_player);
        }
        refresh_player();
        ui_pixel_mascot_jump(s_mascot);
        return;
    }

    if (ev == BSP_BTN_PRESS) {
        ui_pixel_mascot_jump(s_mascot);
    }
}
