// main/rpg_storage.h —— Life RPG 本地持久化接口。
//
// 使用 ESP-IDF NVS,不自行实现底层 Flash 存储。
#pragma once

#include "rpg_player.h"
#include "rpg_quest.h"
#include "rpg_streak.h"
#include "esp_err.h"

// 初始化 NVS。幂等;失败时不自动擦除已有数据。
esp_err_t rpg_storage_init(void);

// 从 NVS 载入玩家数据。数据不存在时保留传入结构体中的默认值。
esp_err_t rpg_storage_load_player(rpg_player_t *player);

// 保存玩家数据到 NVS。
esp_err_t rpg_storage_save_player(const rpg_player_t *player);

// 载入今日任务的完成状态。任务模板由 rpg_quest 模块生成,这里只覆盖完成位。
esp_err_t rpg_storage_load_quests(rpg_quest_t *quests, uint32_t count);

// 保存今日任务的完成状态。
esp_err_t rpg_storage_save_quests(const rpg_quest_t *quests, uint32_t count);

// 载入连续天数状态。
esp_err_t rpg_storage_load_streak(rpg_streak_t *streak);

// 保存连续天数状态。
esp_err_t rpg_storage_save_streak(const rpg_streak_t *streak);

// 只清除 Life RPG 自己的 namespace,不清除 cardid 或其他应用数据。
esp_err_t rpg_storage_reset(void);
