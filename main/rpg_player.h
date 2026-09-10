// main/rpg_player.h —— Life RPG 玩家数据与经验/等级逻辑的公共接口。
//
// 本文件刻意不依赖 ESP-IDF 或 LVGL,只保留纯 C 数据结构和函数,
// 以便在主机上直接编译测试。
#pragma once

#include <stdint.h>

typedef enum {
    RPG_STAT_BODY = 0,      // 身体健康程度,由运动/步行任务提升
    RPG_STAT_CODE,          // 代码能力,由编程/CTF 任务提升
    RPG_STAT_KNOWLEDGE,     // 知识量,由背单词/学习/阅读任务提升
    RPG_STAT_COUNT,
} rpg_stat_t;

typedef struct {
    uint32_t level;       // 当前等级,最小为 1
    uint32_t xp;          // 当前等级内已累计经验
    uint32_t total_xp;    // 角色历史累计经验,升级不清零
    uint16_t stats[RPG_STAT_COUNT]; // BODY / CODE / KNOWLEDGE
} rpg_player_t;

// 把玩家状态重置为 Level 1 / 0 XP / 全属性 0。
void rpg_player_init(rpg_player_t *player);

// 当前等级升到下一级需要多少 XP。经验曲线集中在此函数,避免散落多处。
uint32_t rpg_xp_to_next_level(uint32_t level);

// 把 amount 加入当前 XP 和 total_xp,并连续升级直到 XP 不足。
// 返回本次实际提升的等级数。
uint32_t rpg_player_add_xp(rpg_player_t *player, uint32_t amount);

// 尝试消耗一次升级所需 XP 并提升一级。
// XP 不足时不做任何修改并返回 0;成功时返回新的等级。
uint32_t rpg_player_level_up(rpg_player_t *player);

// 给指定属性增加少量数值。属性只是预留字段,不参与核心等级计算。
void rpg_player_add_stat(rpg_player_t *player, rpg_stat_t stat, uint16_t amount);
