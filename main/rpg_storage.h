// main/rpg_storage.h —— Life RPG 本地持久化接口。
//
// 使用 ESP-IDF NVS,不自行实现底层 Flash 存储。
#pragma once

#include "rpg_player.h"
#include "esp_err.h"

// 初始化 NVS。幂等;失败时不自动擦除已有数据。
esp_err_t rpg_storage_init(void);

// 从 NVS 载入玩家数据。数据不存在时保留传入结构体中的默认值。
esp_err_t rpg_storage_load_player(rpg_player_t *player);

// 保存玩家数据到 NVS。
esp_err_t rpg_storage_save_player(const rpg_player_t *player);

// 只清除 Life RPG 自己的 namespace,不清除 cardid 或其他应用数据。
esp_err_t rpg_storage_reset(void);
