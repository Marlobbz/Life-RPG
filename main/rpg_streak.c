#include "rpg_streak.h"

void rpg_streak_init(rpg_streak_t *streak)
{
    if (!streak) return;
    streak->current_streak = 0;
    streak->last_serial = 0;
}

void rpg_streak_refresh(rpg_streak_t *streak, const rpg_date_t *today)
{
    if (!streak || !today) return;

    uint32_t today_serial = rpg_date_to_serial(today);
    if (streak->last_serial == 0) {
        return;
    }

    // 今天或昨天完成过,连击仍然有效。
    if (today_serial == streak->last_serial ||
        today_serial == streak->last_serial + 1u) {
        return;
    }

    // 时钟回拨或断档超过一天。
    streak->current_streak = 0;
    streak->last_serial = 0;
}

void rpg_streak_on_quest_complete(rpg_streak_t *streak, const rpg_date_t *today)
{
    if (!streak || !today) return;

    uint32_t today_serial = rpg_date_to_serial(today);
    if (today_serial == 0) {
        return;
    }

    if (streak->last_serial == 0 ||
        today_serial != streak->last_serial + 1u) {
        // 首次完成,或今天比上次完成日晚了不止一天,或发生回拨。
        streak->current_streak = 1;
    } else {
        streak->current_streak++;
    }

    streak->last_serial = today_serial;
}

bool rpg_streak_completed_today(const rpg_streak_t *streak, const rpg_date_t *today)
{
    if (!streak || !today) return false;
    return streak->last_serial == rpg_date_to_serial(today);
}
