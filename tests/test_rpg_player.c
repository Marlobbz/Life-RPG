#include <assert.h>
#include "rpg_player.h"

int main(void)
{
    rpg_player_t player;

    rpg_player_init(&player);
    assert(player.level == 1);
    assert(player.xp == 0);
    assert(player.total_xp == 0);
    assert(player.stats[RPG_STAT_BODY] == 0);
    assert(player.stats[RPG_STAT_CODE] == 0);

    assert(rpg_xp_to_next_level(1) == 100);
    assert(rpg_xp_to_next_level(2) == 150);
    assert(rpg_xp_to_next_level(3) == 225);

    assert(rpg_player_add_xp(&player, 20) == 0);
    assert(player.level == 1);
    assert(player.xp == 20);
    assert(player.total_xp == 20);

    assert(rpg_player_add_xp(&player, 80) == 1);
    assert(player.level == 2);
    assert(player.xp == 0);
    assert(player.total_xp == 100);

    assert(rpg_player_level_up(&player) == 0);
    assert(player.level == 2);

    assert(rpg_player_add_xp(&player, 150) == 1);
    assert(player.level == 3);
    assert(player.xp == 0);
    assert(player.total_xp == 250);

    rpg_player_add_stat(&player, RPG_STAT_CODE, 3);
    rpg_player_add_stat(&player, RPG_STAT_KNOWLEDGE, 7);
    assert(player.stats[RPG_STAT_CODE] == 3);
    assert(player.stats[RPG_STAT_BODY] == 0);
    assert(player.stats[RPG_STAT_KNOWLEDGE] == 7);

    return 0;
}
