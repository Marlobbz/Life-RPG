// main/rpg_pet.h —— Life RPG 虚拟宠物公共接口。
#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    RPG_PET_STATE_HAPPY = 0,
    RPG_PET_STATE_NORMAL,
    RPG_PET_STATE_TIRED,
} rpg_pet_state_t;

typedef struct {
    uint8_t mood;   // 0..100
    uint8_t energy; // 0..100
    uint8_t trust;  // 0..100
} rpg_pet_t;

void rpg_pet_init(rpg_pet_t *pet);

// 完成任务时给宠物正向反馈。
void rpg_pet_on_quest_complete(rpg_pet_t *pet);

// 时间流逝导致宠物状态下降。数值均饱和,不会低于 0。
void rpg_pet_decay(rpg_pet_t *pet, uint8_t mood_loss, uint8_t energy_loss);

// 根据当前状态计算宠物表现。
rpg_pet_state_t rpg_pet_state(const rpg_pet_t *pet);

// 返回状态名,方便 UI 直接显示。
const char *rpg_pet_state_name(rpg_pet_state_t state);
