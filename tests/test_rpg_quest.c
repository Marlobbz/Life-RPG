#include <assert.h>
#include "rpg_quest.h"

int main(void)
{
    rpg_player_t player;
    rpg_quest_t quests[RPG_QUEST_MAX_DAILY];
    rpg_date_t date = { 1970, 1, 1 };

    rpg_player_init(&player);
    rpg_quest_init(quests, RPG_QUEST_MAX_DAILY);
    rpg_quest_generate_daily(quests, RPG_QUEST_MAX_DAILY, &date);

    assert(quests[0].xp_reward == 20);
    assert(quests[0].completed == false);

    uint32_t awarded = rpg_quest_complete(&quests[0], &player);
    assert(awarded == 20);
    assert(quests[0].completed == true);
    assert(player.xp == 20);
    assert(player.stats[RPG_STAT_KNOWLEDGE] == 2);

    assert(rpg_quest_complete(&quests[0], &player) == 0);
    assert(player.xp == 20);
    assert(player.stats[RPG_STAT_KNOWLEDGE] == 2);

    assert(rpg_quest_complete(&quests[1], &player) == 40);
    assert(player.xp == 60);
    assert(player.stats[RPG_STAT_BODY] == 3);

    assert(rpg_quest_complete(&quests[2], &player) == 25);
    assert(player.xp == 85);
    assert(player.stats[RPG_STAT_CODE] == 2);

    return 0;
}
