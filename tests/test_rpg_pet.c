#include <assert.h>
#include "rpg_pet.h"

int main(void)
{
    rpg_pet_t pet;

    rpg_pet_init(&pet);
    assert(pet.mood == 70);
    assert(pet.energy == 80);
    assert(pet.trust == 0);
    assert(rpg_pet_state(&pet) == RPG_PET_STATE_HAPPY);

    rpg_pet_decay(&pet, 40, 60);
    assert(pet.mood == 30);
    assert(pet.energy == 20);
    assert(rpg_pet_state(&pet) == RPG_PET_STATE_TIRED);

    rpg_pet_on_quest_complete(&pet);
    assert(pet.mood == 40);
    assert(pet.energy == 25);
    assert(pet.trust == 5);
    assert(rpg_pet_state(&pet) == RPG_PET_STATE_TIRED);

    rpg_pet_init(&pet);
    rpg_pet_decay(&pet, 10, 0);
    assert(pet.mood == 60);
    assert(rpg_pet_state(&pet) == RPG_PET_STATE_NORMAL);

    return 0;
}
