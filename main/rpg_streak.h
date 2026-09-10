// main/rpg_streak.h —— Life RPG 连续天数逻辑。
#pragma once

#include "rpg_date.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t current_streak; // 当前连续完成天数
    uint32_t last_serial;    // 最近一次完成任务的日期序号
} rpg_streak_t;

void rpg_streak_init(rpg_streak_t *streak);

// 根据今天日期刷新 streak;如果已经断档,当前连续天数归零。
void rpg_streak_refresh(rpg_streak_t *streak, const rpg_date_t *today);

// 完成任务时更新 streak;同一天只算一次。
void rpg_streak_on_quest_complete(rpg_streak_t *streak, const rpg_date_t *today);

// 今天是否已经完成过任务。
bool rpg_streak_completed_today(const rpg_streak_t *streak, const rpg_date_t *today);
