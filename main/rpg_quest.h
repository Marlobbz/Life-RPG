// main/rpg_quest.h —— Life RPG 每日任务数据与完成逻辑的公共接口。
//
// Phase 3 使用固定任务模板,不依赖网络或 AI。
#pragma once

#include "rpg_player.h"
#include <stdbool.h>
#include <stdint.h>

#define RPG_QUEST_MAX_DAILY  7u
#define RPG_QUEST_TITLE_MAX  24u
#define RPG_QUEST_DESC_MAX   64u
#define RPG_QUEST_VERSION    3u

typedef struct {
    char title[RPG_QUEST_TITLE_MAX];
    char description[RPG_QUEST_DESC_MAX];
    uint32_t xp_reward;
    rpg_stat_t stat;
    uint16_t stat_amount;
    bool completed;
} rpg_quest_t;

// 把任务数组清空为未完成状态。
void rpg_quest_init(rpg_quest_t *quests, uint32_t count);

// 生成固定任务列表。任务不会再按日期消失或轮换。
void rpg_quest_generate_catalog(rpg_quest_t *quests, uint32_t count);

// 完成任务。已经完成或参数无效时返回 0,不重复发放 XP。
// 成功时返回本次奖励的 XP 数值。
uint32_t rpg_quest_complete(rpg_quest_t *quest, rpg_player_t *player);
