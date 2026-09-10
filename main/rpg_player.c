// main/rpg_player.c —— Life RPG Player / XP / Level 纯逻辑实现。

#include "rpg_player.h"

// Level 1 -> 100 XP, Level 2 -> 150 XP, Level 3 -> 225 XP。
// 之后每级需求乘 1.5,用整数比例避免浮点和平台差异。
#define RPG_XP_BASE        100u
#define RPG_XP_GROWTH_NUM  3u
#define RPG_XP_GROWTH_DEN  2u

static uint32_t sat_add_u32(uint32_t a, uint32_t b)
{
    if (a > UINT32_MAX - b) return UINT32_MAX;
    return a + b;
}

static uint16_t sat_add_u16(uint16_t a, uint16_t b)
{
    if (a > UINT16_MAX - b) return UINT16_MAX;
    return (uint16_t)(a + b);
}

void rpg_player_init(rpg_player_t *player)
{
    if (!player) return;

    player->level = 1;
    player->xp = 0;
    player->total_xp = 0;
    for (int i = 0; i < RPG_STAT_COUNT; i++) {
        player->stats[i] = 0;
    }
}

uint32_t rpg_xp_to_next_level(uint32_t level)
{
    if (level < 1) {
        return RPG_XP_BASE;
    }

    uint32_t needed = RPG_XP_BASE;
    for (uint32_t i = 1; i < level; i++) {
        needed = (needed / RPG_XP_GROWTH_DEN) * RPG_XP_GROWTH_NUM;
    }
    return needed;
}

uint32_t rpg_player_level_up(rpg_player_t *player)
{
    if (!player) return 0;

    uint32_t needed = rpg_xp_to_next_level(player->level);
    if (player->xp < needed) {
        return 0;
    }

    player->xp -= needed;
    player->level++;
    return player->level;
}

uint32_t rpg_player_add_xp(rpg_player_t *player, uint32_t amount)
{
    if (!player) return 0;

    player->xp = sat_add_u32(player->xp, amount);
    player->total_xp = sat_add_u32(player->total_xp, amount);

    uint32_t levels_gained = 0;
    while (rpg_player_level_up(player) != 0) {
        levels_gained++;
    }
    return levels_gained;
}

void rpg_player_add_stat(rpg_player_t *player, rpg_stat_t stat, uint16_t amount)
{
    if (!player || stat < 0 || stat >= RPG_STAT_COUNT) return;
    player->stats[stat] = sat_add_u16(player->stats[stat], amount);
}
