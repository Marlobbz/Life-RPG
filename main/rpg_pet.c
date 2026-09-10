#include "rpg_pet.h"

static uint8_t sat_sub_u8(uint8_t value, uint8_t loss)
{
    return value > loss ? (uint8_t)(value - loss) : 0;
}

static uint8_t sat_add_u8(uint8_t value, uint8_t amount)
{
    uint16_t sum = (uint16_t)value + amount;
    return sum > 100 ? 100 : (uint8_t)sum;
}

void rpg_pet_init(rpg_pet_t *pet)
{
    if (!pet) return;
    pet->mood = 70;
    pet->energy = 80;
    pet->trust = 0;
}

void rpg_pet_on_quest_complete(rpg_pet_t *pet)
{
    if (!pet) return;
    pet->mood = sat_add_u8(pet->mood, 10);
    pet->trust = sat_add_u8(pet->trust, 5);
    pet->energy = sat_add_u8(pet->energy, 5);
}

void rpg_pet_decay(rpg_pet_t *pet, uint8_t mood_loss, uint8_t energy_loss)
{
    if (!pet) return;
    pet->mood = sat_sub_u8(pet->mood, mood_loss);
    pet->energy = sat_sub_u8(pet->energy, energy_loss);
}

rpg_pet_state_t rpg_pet_state(const rpg_pet_t *pet)
{
    if (!pet) return RPG_PET_STATE_TIRED;

    if (pet->energy < 30 || pet->mood < 20) {
        return RPG_PET_STATE_TIRED;
    }
    if (pet->mood >= 70 && pet->energy >= 50) {
        return RPG_PET_STATE_HAPPY;
    }
    return RPG_PET_STATE_NORMAL;
}

const char *rpg_pet_state_name(rpg_pet_state_t state)
{
    switch (state) {
    case RPG_PET_STATE_HAPPY:
        return "HAPPY";
    case RPG_PET_STATE_NORMAL:
        return "NORMAL";
    case RPG_PET_STATE_TIRED:
        return "TIRED";
    default:
        return "UNKNOWN";
    }
}
