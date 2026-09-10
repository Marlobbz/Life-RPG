// main/rpg_storage.c —— 使用 NVS 保存 Life RPG 玩家状态。

#include "rpg_storage.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "rpg_storage";
static const char *RPG_NVS_NAMESPACE = "life_rpg";

static bool s_initialized;

esp_err_t rpg_storage_init(void)
{
    if (s_initialized) {
        return ESP_OK;
    }

    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        // 和官方 demo_radio 一致:失败时保留用户已有数据,不自动 erase。
        ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(err));
        return err;
    }

    s_initialized = true;
    return ESP_OK;
}

esp_err_t rpg_storage_load_player(rpg_player_t *player)
{
    if (!player) return ESP_ERR_INVALID_ARG;

    rpg_player_init(player);

    esp_err_t err = rpg_storage_init();
    if (err != ESP_OK) {
        return err;
    }

    nvs_handle_t handle;
    err = nvs_open(RPG_NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return ESP_ERR_NVS_NOT_FOUND;
    }
    if (err != ESP_OK) {
        return err;
    }

    uint32_t value_u32 = 0;
    uint16_t value_u16 = 0;

    if (nvs_get_u32(handle, "lvl", &value_u32) == ESP_OK) {
        player->level = (value_u32 < 1) ? 1 : value_u32;
    }
    if (nvs_get_u32(handle, "xp", &value_u32) == ESP_OK) {
        player->xp = value_u32;
    }
    if (nvs_get_u32(handle, "total_xp", &value_u32) == ESP_OK) {
        player->total_xp = value_u32;
    }

    const char *stat_keys[RPG_STAT_COUNT] = {
        "body", "code", "know",
    };
    for (int i = 0; i < RPG_STAT_COUNT; i++) {
        if (nvs_get_u16(handle, stat_keys[i], &value_u16) == ESP_OK) {
            player->stats[i] = value_u16;
        }
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t rpg_storage_save_player(const rpg_player_t *player)
{
    if (!player) return ESP_ERR_INVALID_ARG;

    esp_err_t err = rpg_storage_init();
    if (err != ESP_OK) {
        return err;
    }

    nvs_handle_t handle;
    err = nvs_open(RPG_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_u32(handle, "lvl", player->level);
    if (err == ESP_OK) err = nvs_set_u32(handle, "xp", player->xp);
    if (err == ESP_OK) err = nvs_set_u32(handle, "total_xp", player->total_xp);

    const char *stat_keys[RPG_STAT_COUNT] = {
        "body", "code", "know",
    };
    for (int i = 0; i < RPG_STAT_COUNT && err == ESP_OK; i++) {
        err = nvs_set_u16(handle, stat_keys[i], player->stats[i]);
    }

    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}

esp_err_t rpg_storage_load_quests(rpg_quest_t *quests, uint32_t count,
                                  const rpg_date_t *today)
{
    if (!quests || !today) return ESP_ERR_INVALID_ARG;
    if (count > RPG_QUEST_MAX_DAILY) {
        count = RPG_QUEST_MAX_DAILY;
    }

    esp_err_t err = rpg_storage_init();
    if (err != ESP_OK) {
        return err;
    }

    nvs_handle_t handle;
    err = nvs_open(RPG_NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return ESP_ERR_NVS_NOT_FOUND;
    }
    if (err != ESP_OK) {
        return err;
    }

    uint32_t quest_version = 0;
    uint32_t quest_day = 0;
    if (nvs_get_u32(handle, "q_ver", &quest_version) != ESP_OK ||
        quest_version != RPG_QUEST_VERSION ||
        nvs_get_u32(handle, "q_day", &quest_day) != ESP_OK ||
        quest_day != rpg_date_to_serial(today)) {
        nvs_close(handle);
        return ESP_ERR_NVS_NOT_FOUND;
    }

    for (uint32_t i = 0; i < count; i++) {
        char key[17];
        snprintf(key, sizeof(key), "q_done%u", (unsigned)i);

        uint8_t done = 0;
        if (nvs_get_u8(handle, key, &done) == ESP_OK) {
            quests[i].completed = (done != 0);
        }
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t rpg_storage_save_quests(const rpg_quest_t *quests, uint32_t count,
                                  const rpg_date_t *today)
{
    if (!quests || !today) return ESP_ERR_INVALID_ARG;
    if (count > RPG_QUEST_MAX_DAILY) {
        count = RPG_QUEST_MAX_DAILY;
    }

    esp_err_t err = rpg_storage_init();
    if (err != ESP_OK) {
        return err;
    }

    nvs_handle_t handle;
    err = nvs_open(RPG_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_u32(handle, "q_ver", RPG_QUEST_VERSION);
    if (err == ESP_OK) {
        err = nvs_set_u32(handle, "q_day", rpg_date_to_serial(today));
    }
    if (err != ESP_OK) {
        nvs_close(handle);
        return err;
    }

    for (uint32_t i = 0; i < count; i++) {
        char key[17];
        snprintf(key, sizeof(key), "q_done%u", (unsigned)i);
        err = nvs_set_u8(handle, key, quests[i].completed ? 1 : 0);
        if (err != ESP_OK) {
            break;
        }
    }

    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}

esp_err_t rpg_storage_load_streak(rpg_streak_t *streak)
{
    if (!streak) return ESP_ERR_INVALID_ARG;

    rpg_streak_init(streak);
    esp_err_t err = rpg_storage_init();
    if (err != ESP_OK) {
        return err;
    }

    nvs_handle_t handle;
    err = nvs_open(RPG_NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return ESP_ERR_NVS_NOT_FOUND;
    }
    if (err != ESP_OK) {
        return err;
    }

    uint32_t value = 0;
    if (nvs_get_u32(handle, "streak", &value) == ESP_OK) {
        streak->current_streak = value;
    }
    if (nvs_get_u32(handle, "streak_last", &value) == ESP_OK) {
        streak->last_serial = value;
    }

    nvs_close(handle);
    return ESP_OK;
}

esp_err_t rpg_storage_save_streak(const rpg_streak_t *streak)
{
    if (!streak) return ESP_ERR_INVALID_ARG;

    esp_err_t err = rpg_storage_init();
    if (err != ESP_OK) {
        return err;
    }

    nvs_handle_t handle;
    err = nvs_open(RPG_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_u32(handle, "streak", streak->current_streak);
    if (err == ESP_OK) {
        err = nvs_set_u32(handle, "streak_last", streak->last_serial);
    }
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }

    nvs_close(handle);
    return err;
}

esp_err_t rpg_storage_reset(void)
{
    esp_err_t err = rpg_storage_init();
    if (err != ESP_OK) {
        return err;
    }

    nvs_handle_t handle;
    err = nvs_open(RPG_NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return ESP_OK;
    }
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_erase_all(handle);
    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);
    return err;
}
