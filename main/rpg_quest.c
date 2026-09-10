// main/rpg_quest.c —— 固定任务模板和完成判定。

#include "rpg_quest.h"
#include <string.h>

static const rpg_quest_t TEMPLATES[] = {
    {
        .title = "Memorize 30 words",
        .description = "Memorize thirty new words.",
        .xp_reward = 20,
        .stat = RPG_STAT_KNOWLEDGE,
        .stat_amount = 2,
    },
    {
        .title = "Exercise 2 hours",
        .description = "Work out for two hours.",
        .xp_reward = 40,
        .stat = RPG_STAT_BODY,
        .stat_amount = 3,
    },
    {
        .title = "Code 1 problem",
        .description = "Solve one programming problem.",
        .xp_reward = 25,
        .stat = RPG_STAT_CODE,
        .stat_amount = 2,
    },
    {
        .title = "Study 30 min",
        .description = "Focus on one subject for thirty minutes.",
        .xp_reward = 20,
        .stat = RPG_STAT_KNOWLEDGE,
        .stat_amount = 1,
    },
    {
        .title = "Read 10 pages",
        .description = "Read ten pages of a book.",
        .xp_reward = 10,
        .stat = RPG_STAT_KNOWLEDGE,
        .stat_amount = 1,
    },
    {
        .title = "Walk 3000 steps",
        .description = "Walk three thousand steps.",
        .xp_reward = 10,
        .stat = RPG_STAT_BODY,
        .stat_amount = 1,
    },
    {
        .title = "Solve 1 CTF",
        .description = "Complete one security challenge.",
        .xp_reward = 30,
        .stat = RPG_STAT_CODE,
        .stat_amount = 2,
    },
};

#define TEMPLATE_COUNT (sizeof(TEMPLATES) / sizeof(TEMPLATES[0]))

void rpg_quest_init(rpg_quest_t *quests, uint32_t count)
{
    if (!quests) return;
    if (count > RPG_QUEST_MAX_DAILY) {
        count = RPG_QUEST_MAX_DAILY;
    }

    for (uint32_t i = 0; i < count; i++) {
        memset(quests[i].title, 0, sizeof(quests[i].title));
        memset(quests[i].description, 0, sizeof(quests[i].description));
        quests[i].xp_reward = 0;
        quests[i].stat = RPG_STAT_BODY;
        quests[i].stat_amount = 0;
        quests[i].completed = false;
    }
}

void rpg_quest_generate_daily(rpg_quest_t *quests, uint32_t count,
                              const rpg_date_t *date)
{
    if (!quests || !date) return;
    if (count > RPG_QUEST_MAX_DAILY) {
        count = RPG_QUEST_MAX_DAILY;
    }

    uint32_t start = rpg_date_to_serial(date) % TEMPLATE_COUNT;
    for (uint32_t i = 0; i < count; i++) {
        quests[i] = TEMPLATES[(start + i) % TEMPLATE_COUNT];
    }
}

uint32_t rpg_quest_complete(rpg_quest_t *quest, rpg_player_t *player)
{
    if (!quest || !player || quest->completed) {
        return 0;
    }

    quest->completed = true;
    if (quest->stat >= 0 && quest->stat < RPG_STAT_COUNT && quest->stat_amount > 0) {
        rpg_player_add_stat(player, quest->stat, quest->stat_amount);
    }

    rpg_player_add_xp(player, quest->xp_reward);
    return quest->xp_reward;
}
