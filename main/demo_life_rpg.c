// main/demo_life_rpg.c —— AI Passport Life RPG 页面入口。
//
// Phase 3 阶段在页面内加入一个很小的状态机:
//   HOME -> PLAYER
//   HOME -> QUEST LIST -> QUEST DETAIL
//
// 纯逻辑分别放在 rpg_player.c / rpg_quest.c / rpg_storage.c,
// 本文件只负责 LVGL 页面和按键分发。

#include "demo.h"
#include "bsp_battery.h"
#include "rpg_player.h"
#include "rpg_quest.h"
#include "rpg_date.h"
#include "rpg_streak.h"
#include "rpg_pet.h"
#include "rpg_audio.h"
#include "rpg_storage.h"
#include "ui_pixel.h"
#include "esp_log.h"
#include "lvgl.h"
#include <stdio.h>

static const char *TAG = "life_rpg";

typedef enum {
    LIFE_VIEW_HOME = 0,
    LIFE_VIEW_PLAYER,
    LIFE_VIEW_QUEST_LIST,
    LIFE_VIEW_QUEST_DETAIL,
    LIFE_VIEW_PET,
} life_view_t;

static lv_obj_t *s_scr;
static lv_obj_t *s_battery;
static lv_obj_t *s_level;
static lv_obj_t *s_xp;
static lv_obj_t *s_xp_bar;
static lv_obj_t *s_stats;
static lv_obj_t *s_streak_label;
static lv_obj_t *s_home_cards[3];
static lv_obj_t *s_pet_mood;
static lv_obj_t *s_pet_energy;
static lv_obj_t *s_pet_trust;
static lv_obj_t *s_pet_state;
static lv_obj_t *s_quest_rows[RPG_QUEST_MAX_DAILY];
static lv_obj_t *s_quest_row_labels[RPG_QUEST_MAX_DAILY];
static lv_obj_t *s_quest_title;
static lv_obj_t *s_quest_desc;
static lv_obj_t *s_quest_reward;
static lv_obj_t *s_quest_status;
static lv_obj_t *s_mascot;

static rpg_player_t s_player;
static rpg_quest_t s_quests[RPG_QUEST_MAX_DAILY];
static rpg_streak_t s_streak;
static rpg_pet_t s_pet;
static uint32_t s_pet_last_serial;
static rpg_date_t s_dev_date;
static life_view_t s_view;
static int s_home_sel;
static int s_quest_sel;
static int s_mascot_base_y;

// 开发模式日期来源。后续接入 RTC/SNTP 时只替换 provider,业务代码不用改。
static void life_rpg_date_provider(rpg_date_t *out)
{
    if (out) {
        *out = s_dev_date;
    }
}

static void refresh_battery(void)
{
    if (!s_battery) return;

    int soc = bsp_battery_soc();
    if (soc < 0) {
        lv_label_set_text(s_battery, "BATT --");
        lv_obj_set_style_text_color(s_battery, lv_color_hex(UI_MUTED), 0);
        return;
    }

    lv_label_set_text_fmt(s_battery, "BATT %d%%", soc);
    lv_obj_set_style_text_color(s_battery,
                                (soc < 20) ? lv_color_hex(0xFF5A5A)
                                           : lv_color_hex(UI_INK),
                                0);
}

static void add_battery(lv_obj_t *parent)
{
    s_battery = lv_label_create(parent);
    lv_obj_set_style_text_font(s_battery, &lv_font_montserrat_14, 0);
    lv_obj_set_pos(s_battery, 174, 27);
    refresh_battery();
}

static void clear_screen(void)
{
    if (s_scr) {
        lv_obj_delete(s_scr);
        s_scr = NULL;
    }

    s_battery = NULL;
    s_level = NULL;
    s_xp = NULL;
    s_xp_bar = NULL;
    s_stats = NULL;
    s_streak_label = NULL;
    s_pet_mood = NULL;
    s_pet_energy = NULL;
    s_pet_trust = NULL;
    s_pet_state = NULL;
    s_mascot = NULL;
    s_quest_title = NULL;
    s_quest_desc = NULL;
    s_quest_reward = NULL;
    s_quest_status = NULL;

    for (int i = 0; i < 3; i++) {
        s_home_cards[i] = NULL;
    }
    for (uint32_t i = 0; i < RPG_QUEST_MAX_DAILY; i++) {
        s_quest_rows[i] = NULL;
        s_quest_row_labels[i] = NULL;
    }
}

static void life_rpg_jump_mascot(void)
{
    if (!s_mascot) return;

    lv_obj_set_y(s_mascot, s_mascot_base_y);
    ui_pixel_mascot_jump(s_mascot);
}

static void load_screen(void)
{
    if (!s_scr) return;
    lv_scr_load_anim(s_scr, LV_SCR_LOAD_ANIM_FADE_ON, 180, 0, false);
}

static void refresh_player(void)
{
    if (!s_level || !s_xp || !s_stats) return;

    lv_label_set_text_fmt(s_level, "LV.%02u", (unsigned)s_player.level);
    lv_label_set_text_fmt(s_xp, "XP %u / %u",
                          (unsigned)s_player.xp,
                          (unsigned)rpg_xp_to_next_level(s_player.level));
    if (s_xp_bar) {
        uint32_t needed = rpg_xp_to_next_level(s_player.level);
        lv_bar_set_range(s_xp_bar, 0, (int32_t)needed);
        lv_bar_set_value(s_xp_bar, (int32_t)s_player.xp, LV_ANIM_ON);
    }
    lv_label_set_text_fmt(s_stats, "BODY %u  CODE %u\nKNOWLEDGE %u",
                          (unsigned)s_player.stats[RPG_STAT_BODY],
                          (unsigned)s_player.stats[RPG_STAT_CODE],
                          (unsigned)s_player.stats[RPG_STAT_KNOWLEDGE]);

    if (s_streak_label) {
        rpg_date_t today;
        rpg_date_today(&today);
        bool done_today = rpg_streak_completed_today(&s_streak, &today);
        lv_label_set_text_fmt(s_streak_label, "STREAK %u DAYS  %s",
                              (unsigned)s_streak.current_streak,
                              done_today ? "DONE" : "OPEN");
    }
}

static void refresh_pet(void)
{
    if (!s_pet_mood || !s_pet_energy || !s_pet_trust || !s_pet_state) return;

    lv_label_set_text_fmt(s_pet_mood, "MOOD %u", (unsigned)s_pet.mood);
    lv_label_set_text_fmt(s_pet_energy, "ENERGY %u", (unsigned)s_pet.energy);
    lv_label_set_text_fmt(s_pet_trust, "TRUST %u", (unsigned)s_pet.trust);
    lv_label_set_text_fmt(s_pet_state, "STATE: %s",
                          rpg_pet_state_name(rpg_pet_state(&s_pet)));
}

static void fx_fade(void *obj, int32_t value)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)value, 0);
}

static void anim_y(void *obj, int32_t value)
{
    lv_obj_set_y((lv_obj_t *)obj, value);
}

static void play_level_up_feedback(void)
{
    if (!s_scr) return;

    lv_obj_t *label = lv_label_create(s_scr);
    lv_label_set_text(label, "LEVEL UP!");
    lv_obj_set_width(label, 240);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(UI_YELLOW), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_pos(label, 0, 105);
    lv_obj_set_style_opa(label, LV_OPA_COVER, 0);

    lv_anim_t move;
    lv_anim_init(&move);
    lv_anim_set_var(&move, label);
    lv_anim_set_exec_cb(&move, anim_y);
    lv_anim_set_values(&move, 105, 60);
    lv_anim_set_duration(&move, 700);
    lv_anim_set_path_cb(&move, lv_anim_path_ease_out);
    lv_anim_start(&move);

    lv_anim_t fade;
    lv_anim_init(&fade);
    lv_anim_set_var(&fade, label);
    lv_anim_set_exec_cb(&fade, fx_fade);
    lv_anim_set_values(&fade, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_duration(&fade, 1000);
    lv_anim_set_path_cb(&fade, lv_anim_path_linear);
    lv_anim_start(&fade);
}

static void play_completion_feedback(void)
{
    if (!s_scr) return;

    static const uint32_t COLORS[] = {
        UI_YELLOW, UI_ORANGE, UI_RED, 0x39FF88, 0x1689E8, 0xFFB23E,
    };
    static const int OFFSETS[][2] = {
        { -42, -28 }, {  44, -32 }, { -52,  22 },
        {  54,  24 }, { -24, -48 }, {  28,  46 },
    };

    for (int i = 0; i < (int)(sizeof(COLORS) / sizeof(COLORS[0])); i++) {
        lv_obj_t *p = lv_obj_create(s_scr);
        lv_obj_remove_flag(p, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_pos(p, 120 + OFFSETS[i][0], 150 + OFFSETS[i][1]);
        lv_obj_set_size(p, 7, 7);
        lv_obj_set_style_radius(p, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(p, 0, 0);
        lv_obj_set_style_bg_color(p, lv_color_hex(COLORS[i]), 0);
        lv_obj_set_style_opa(p, LV_OPA_COVER, 0);

        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, p);
        lv_anim_set_exec_cb(&a, fx_fade);
        lv_anim_set_values(&a, LV_OPA_COVER, LV_OPA_TRANSP);
        lv_anim_set_duration(&a, 900);
        lv_anim_set_path_cb(&a, lv_anim_path_linear);
        lv_anim_start(&a);
    }
}

static void refresh_home(void)
{
    for (int i = 0; i < 3; i++) {
        if (s_home_cards[i]) {
            ui_pixel_set_selected(s_home_cards[i], i == s_home_sel, true);
        }
    }
}

static void refresh_quest_list(void)
{
    int top = s_quest_sel - 1;
    if (top < 0) top = 0;
    int max_top = (int)RPG_QUEST_MAX_DAILY - 3;
    if (top > max_top) top = max_top;

    for (uint32_t i = 0; i < RPG_QUEST_MAX_DAILY; i++) {
        if (i >= 3) break;
        if (!s_quest_rows[i] || !s_quest_row_labels[i]) continue;

        uint32_t quest_index = (uint32_t)(top + (int)i);
        if (quest_index >= RPG_QUEST_MAX_DAILY) continue;

        char line[RPG_QUEST_TITLE_MAX + 8];
        snprintf(line, sizeof(line), "[%c] %s",
                 s_quests[quest_index].completed ? 'X' : ' ',
                 s_quests[quest_index].title);
        lv_label_set_text(s_quest_row_labels[i], line);
        ui_pixel_set_selected(s_quest_rows[i], (int)quest_index == s_quest_sel, true);
    }
}

static void refresh_quest_detail(void)
{
    if (!s_quest_title || !s_quest_desc || !s_quest_reward || !s_quest_status) {
        return;
    }

    lv_label_set_text(s_quest_title, s_quests[s_quest_sel].title);
    lv_label_set_text(s_quest_desc, s_quests[s_quest_sel].description);
    lv_label_set_text_fmt(s_quest_reward, "+%u XP",
                          (unsigned)s_quests[s_quest_sel].xp_reward);
    lv_label_set_text(s_quest_status,
                      s_quests[s_quest_sel].completed ? "COMPLETE"
                                                      : "NOT COMPLETE");
}

static void build_home(void)
{
    clear_screen();

    s_scr = ui_pixel_screen_create("LIFE RPG");
    add_battery(s_scr);

    static const char *HOME_NAMES[] = { "PLAYER", "QUEST", "PET" };
    for (int i = 0; i < 3; i++) {
        int y = 72 + i * 50;
        s_home_cards[i] = ui_pixel_panel_create(s_scr, 25, y, 190, 42, UI_PAPER);

        lv_obj_t *label = lv_label_create(s_home_cards[i]);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(UI_INK), 0);
        lv_label_set_text(label, HOME_NAMES[i]);
        lv_obj_center(label);
    }

    lv_obj_t *hint = lv_label_create(s_scr);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(UI_INK), 0);
    lv_label_set_text(hint, "UP/DOWN SELECT  OK ENTER");
    lv_obj_set_pos(hint, 20, 218);

    s_mascot = ui_pixel_mascot_create(s_scr, 101, 238);
    s_mascot_base_y = 238;
    refresh_home();
    load_screen();
}

static void build_player(void)
{
    clear_screen();

    s_scr = ui_pixel_screen_create("PLAYER");
    add_battery(s_scr);

    lv_obj_t *panel = ui_pixel_panel_create(s_scr, 12, 58, 216, 172, UI_PAPER);

    s_level = lv_label_create(panel);
    lv_obj_set_style_text_font(s_level, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(s_level, lv_color_hex(UI_INK), 0);
    lv_obj_align(s_level, LV_ALIGN_TOP_MID, 0, 6);

    s_xp = lv_label_create(panel);
    lv_obj_set_style_text_font(s_xp, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_xp, lv_color_hex(UI_SKY_DARK), 0);
    lv_obj_align(s_xp, LV_ALIGN_TOP_MID, 0, 28);

    s_xp_bar = lv_bar_create(panel);
    lv_obj_set_size(s_xp_bar, 176, 10);
    lv_obj_align(s_xp_bar, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_radius(s_xp_bar, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_radius(s_xp_bar, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(s_xp_bar, lv_color_hex(UI_MUTED), LV_PART_MAIN);
    lv_obj_set_style_bg_color(s_xp_bar, lv_color_hex(UI_GRASS), LV_PART_INDICATOR);

    s_streak_label = lv_label_create(panel);
    lv_obj_set_style_text_font(s_streak_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_streak_label, lv_color_hex(UI_RED), 0);
    lv_obj_align(s_streak_label, LV_ALIGN_TOP_MID, 0, 68);

    s_stats = lv_label_create(panel);
    lv_obj_set_style_text_font(s_stats, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_stats, lv_color_hex(UI_INK), 0);
    lv_obj_align(s_stats, LV_ALIGN_TOP_MID, 0, 92);

    lv_obj_t *hint = lv_label_create(panel);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(UI_SKY_DARK), 0);
    lv_label_set_text(hint, "UP/DOWN: DATE  DBL: BACK");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -6);

    s_mascot = ui_pixel_mascot_create(s_scr, 101, 238);
    s_mascot_base_y = 238;
    refresh_player();
    load_screen();
}

static void build_quest_list(void)
{
    clear_screen();

    s_scr = ui_pixel_screen_create("QUESTS");
    add_battery(s_scr);

    lv_obj_t *panel = ui_pixel_panel_create(s_scr, 16, 66, 208, 158, UI_PAPER);

    for (uint32_t i = 0; i < RPG_QUEST_MAX_DAILY; i++) {
        int y = 8 + (int)i * 50;
        s_quest_rows[i] = ui_pixel_panel_create(panel, 8, y, 176, 42, UI_PAPER);
        s_quest_row_labels[i] = lv_label_create(s_quest_rows[i]);
        lv_obj_set_style_text_font(s_quest_row_labels[i], &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(s_quest_row_labels[i], lv_color_hex(UI_INK), 0);
        lv_obj_center(s_quest_row_labels[i]);
    }

    lv_obj_t *hint = lv_label_create(s_scr);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(UI_INK), 0);
    lv_label_set_text(hint, "UP/DOWN SELECT  OK DETAIL  DBL BACK");
    lv_obj_set_pos(hint, 16, 236);

    refresh_quest_list();
    load_screen();
}

static void build_quest_detail(void)
{
    clear_screen();

    s_scr = ui_pixel_screen_create("QUEST");
    add_battery(s_scr);

    lv_obj_t *panel = ui_pixel_panel_create(s_scr, 16, 66, 208, 158, UI_PAPER);

    s_quest_title = lv_label_create(panel);
    lv_obj_set_style_text_font(s_quest_title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(s_quest_title, lv_color_hex(UI_INK), 0);
    lv_obj_align(s_quest_title, LV_ALIGN_TOP_MID, 0, 8);

    s_quest_desc = lv_label_create(panel);
    lv_obj_set_style_text_font(s_quest_desc, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_quest_desc, lv_color_hex(UI_SKY_DARK), 0);
    lv_obj_set_width(s_quest_desc, 176);
    lv_label_set_long_mode(s_quest_desc, LV_LABEL_LONG_WRAP);
    lv_obj_align(s_quest_desc, LV_ALIGN_TOP_MID, 0, 40);

    s_quest_reward = lv_label_create(panel);
    lv_obj_set_style_text_font(s_quest_reward, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_quest_reward, lv_color_hex(UI_GRASS_DARK), 0);
    lv_obj_align(s_quest_reward, LV_ALIGN_TOP_MID, 0, 82);

    s_quest_status = lv_label_create(panel);
    lv_obj_set_style_text_font(s_quest_status, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_quest_status, lv_color_hex(UI_INK), 0);
    lv_obj_align(s_quest_status, LV_ALIGN_BOTTOM_MID, 0, -8);

    s_mascot = ui_pixel_mascot_create(s_scr, 101, 238);
    s_mascot_base_y = 238;
    refresh_quest_detail();
    load_screen();
}

static void build_pet(void)
{
    clear_screen();

    s_scr = ui_pixel_screen_create("PET");
    add_battery(s_scr);

    lv_obj_t *panel = ui_pixel_panel_create(s_scr, 12, 58, 216, 172, UI_PAPER);

    s_pet_mood = lv_label_create(panel);
    lv_obj_set_style_text_font(s_pet_mood, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(s_pet_mood, lv_color_hex(UI_INK), 0);
    lv_obj_align(s_pet_mood, LV_ALIGN_TOP_MID, 0, 8);

    s_pet_energy = lv_label_create(panel);
    lv_obj_set_style_text_font(s_pet_energy, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_pet_energy, lv_color_hex(UI_SKY_DARK), 0);
    lv_obj_align(s_pet_energy, LV_ALIGN_TOP_MID, 0, 42);

    s_pet_trust = lv_label_create(panel);
    lv_obj_set_style_text_font(s_pet_trust, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_pet_trust, lv_color_hex(UI_INK), 0);
    lv_obj_align(s_pet_trust, LV_ALIGN_TOP_MID, 0, 68);

    s_pet_state = lv_label_create(panel);
    lv_obj_set_style_text_font(s_pet_state, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(s_pet_state, lv_color_hex(UI_ORANGE), 0);
    lv_obj_align(s_pet_state, LV_ALIGN_TOP_MID, 0, 96);

    lv_obj_t *hint = lv_label_create(panel);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(hint, lv_color_hex(UI_SKY_DARK), 0);
    lv_label_set_text(hint, "DBL: BACK");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -6);

    s_mascot = ui_pixel_mascot_create(s_scr, 101, 238);
    s_mascot_base_y = 238;
    refresh_pet();
    load_screen();
}

void demo_life_rpg_enter(void)
{
    rpg_player_init(&s_player);
    rpg_quest_init(s_quests, RPG_QUEST_MAX_DAILY);
    rpg_streak_init(&s_streak);
    rpg_pet_init(&s_pet);
    s_pet_last_serial = 0;

    // Phase 4 开发模式日期。正式 RTC/SNTP 接入后替换 provider 即可。
    s_dev_date.year = 2026;
    s_dev_date.month = 9;
    s_dev_date.day = 10;
    rpg_date_set_provider(life_rpg_date_provider);

    rpg_date_t today;
    rpg_date_today(&today);
    rpg_quest_generate_catalog(s_quests, RPG_QUEST_MAX_DAILY);

    esp_err_t storage_err = rpg_storage_init();
    if (storage_err == ESP_OK) {
        storage_err = rpg_storage_load_player(&s_player);
        if (storage_err != ESP_OK) {
            ESP_LOGW(TAG, "player load failed, using default player: %s",
                     esp_err_to_name(storage_err));
        }

        storage_err = rpg_storage_load_quests(s_quests, RPG_QUEST_MAX_DAILY);
        if (storage_err != ESP_OK) {
            ESP_LOGW(TAG, "quest load failed, using generated quests: %s",
                     esp_err_to_name(storage_err));
        }

        storage_err = rpg_storage_load_streak(&s_streak);
        if (storage_err != ESP_OK) {
            ESP_LOGW(TAG, "streak load failed, using default streak: %s",
                     esp_err_to_name(storage_err));
        }

        storage_err = rpg_storage_load_pet(&s_pet, &s_pet_last_serial);
        if (storage_err != ESP_OK) {
            ESP_LOGW(TAG, "pet load failed, using default pet: %s",
                     esp_err_to_name(storage_err));
        }
    } else {
        ESP_LOGW(TAG, "storage init failed, using defaults: %s",
                 esp_err_to_name(storage_err));
    }

    rpg_streak_refresh(&s_streak, &today);

    uint32_t today_serial = rpg_date_to_serial(&today);
    if (s_pet_last_serial == 0) {
        s_pet_last_serial = today_serial;
    } else if (today_serial > s_pet_last_serial) {
        uint32_t missed = today_serial - s_pet_last_serial;
        if (missed > 10) {
            missed = 10;
        }
        rpg_pet_decay(&s_pet, (uint8_t)(missed * 5), (uint8_t)(missed * 10));
        s_pet_last_serial = today_serial;
        rpg_storage_save_pet(&s_pet, s_pet_last_serial);
    }

    rpg_audio_start();

    s_view = LIFE_VIEW_HOME;
    s_home_sel = 0;
    s_quest_sel = 0;
    build_home();
}

void demo_life_rpg_exit(void)
{
    rpg_date_t today;
    rpg_date_today(&today);
    rpg_storage_save_player(&s_player);
    rpg_storage_save_quests(s_quests, RPG_QUEST_MAX_DAILY);
    rpg_storage_save_streak(&s_streak);
    rpg_storage_save_pet(&s_pet, s_pet_last_serial);
    rpg_audio_stop();
    clear_screen();
}

void demo_life_rpg_key(bsp_btn_t btn, bsp_btn_ev_t ev)
{
    if (ev == BSP_BTN_DOUBLE) {
        if (s_view == LIFE_VIEW_PLAYER || s_view == LIFE_VIEW_QUEST_LIST ||
            s_view == LIFE_VIEW_PET) {
            s_view = LIFE_VIEW_HOME;
            build_home();
        } else if (s_view == LIFE_VIEW_QUEST_DETAIL) {
            s_view = LIFE_VIEW_QUEST_LIST;
            build_quest_list();
        }
        return;
    }

    switch (s_view) {
    case LIFE_VIEW_HOME:
        if (ev != BSP_BTN_CLICK) return;
        if (btn == BSP_BTN_UP || btn == BSP_BTN_DOWN) {
            s_home_sel = (s_home_sel + 1) % 3;
            refresh_home();
            life_rpg_jump_mascot();
        } else if (btn == BSP_BTN_OK) {
            if (s_home_sel == 0) {
                s_view = LIFE_VIEW_PLAYER;
                build_player();
            } else if (s_home_sel == 1) {
                s_view = LIFE_VIEW_QUEST_LIST;
                s_quest_sel = 0;
                build_quest_list();
            } else {
                s_view = LIFE_VIEW_PET;
                build_pet();
            }
        }
        break;

    case LIFE_VIEW_PLAYER:
        if (ev == BSP_BTN_CLICK && (btn == BSP_BTN_UP || btn == BSP_BTN_DOWN)) {
            int delta = (btn == BSP_BTN_UP) ? 1 : -1;
            int day = (int)s_dev_date.day + delta;
            if (day < 1) day += 31;
            if (day > 31) day -= 31;
            s_dev_date.day = (uint8_t)day;

            rpg_date_t today;
            rpg_date_today(&today);
            rpg_streak_refresh(&s_streak, &today);
            rpg_quest_generate_catalog(s_quests, RPG_QUEST_MAX_DAILY);
            rpg_storage_save_quests(s_quests, RPG_QUEST_MAX_DAILY);
            rpg_storage_save_streak(&s_streak);

            uint32_t today_serial = rpg_date_to_serial(&today);
            if (today_serial > s_pet_last_serial) {
                uint32_t missed = today_serial - s_pet_last_serial;
                if (missed > 10) missed = 10;
                rpg_pet_decay(&s_pet, (uint8_t)(missed * 5), (uint8_t)(missed * 10));
                s_pet_last_serial = today_serial;
                rpg_storage_save_pet(&s_pet, s_pet_last_serial);
            }
            refresh_player();
            life_rpg_jump_mascot();
        }
        break;

    case LIFE_VIEW_QUEST_LIST:
        if (ev != BSP_BTN_CLICK) return;
        if (btn == BSP_BTN_UP) {
            s_quest_sel = (s_quest_sel + RPG_QUEST_MAX_DAILY - 1) % RPG_QUEST_MAX_DAILY;
            refresh_quest_list();
        } else if (btn == BSP_BTN_DOWN) {
            s_quest_sel = (s_quest_sel + 1) % RPG_QUEST_MAX_DAILY;
            refresh_quest_list();
        } else if (btn == BSP_BTN_OK) {
            s_view = LIFE_VIEW_QUEST_DETAIL;
            build_quest_detail();
        }
        break;

    case LIFE_VIEW_QUEST_DETAIL:
        if (ev != BSP_BTN_CLICK) return;
        if (btn == BSP_BTN_OK) {
            uint32_t old_level = s_player.level;
            uint32_t awarded = rpg_quest_complete(&s_quests[s_quest_sel], &s_player);
            if (awarded > 0) {
                uint32_t levels_gained = s_player.level - old_level;
                rpg_date_t today;
                rpg_date_today(&today);
                rpg_streak_on_quest_complete(&s_streak, &today);
                rpg_pet_on_quest_complete(&s_pet);
                s_pet_last_serial = rpg_date_to_serial(&today);
                rpg_storage_save_player(&s_player);
                rpg_storage_save_quests(s_quests, RPG_QUEST_MAX_DAILY);
                rpg_storage_save_streak(&s_streak);
                rpg_storage_save_pet(&s_pet, s_pet_last_serial);
                if (s_quest_status) {
                    lv_label_set_text_fmt(s_quest_status, "QUEST COMPLETE +%u XP",
                                          (unsigned)awarded);
                }
                play_completion_feedback();
                if (levels_gained > 0) {
                    play_level_up_feedback();
                    rpg_audio_play_tone(1200, 180);
                } else {
                    rpg_audio_play_tone(800, 100);
                }
            }
            life_rpg_jump_mascot();
        }
        break;

    case LIFE_VIEW_PET:
        (void)btn;
        (void)ev;
        break;
    }
}
