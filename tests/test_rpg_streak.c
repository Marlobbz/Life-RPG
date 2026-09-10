#include <assert.h>
#include "rpg_streak.h"

static void test_streak(void)
{
    rpg_streak_t streak;
    rpg_date_t day;

    rpg_streak_init(&streak);

    day.year = 2026; day.month = 9; day.day = 10;
    rpg_streak_refresh(&streak, &day);
    assert(streak.current_streak == 0);

    rpg_streak_on_quest_complete(&streak, &day);
    assert(streak.current_streak == 1);
    assert(rpg_streak_completed_today(&streak, &day));

    rpg_streak_on_quest_complete(&streak, &day);
    assert(streak.current_streak == 1);

    day.day = 11;
    rpg_streak_on_quest_complete(&streak, &day);
    assert(streak.current_streak == 2);

    day.day = 12;
    rpg_streak_refresh(&streak, &day);
    assert(streak.current_streak == 2);

    day.day = 13;
    rpg_streak_refresh(&streak, &day);
    assert(streak.current_streak == 0);

    rpg_streak_on_quest_complete(&streak, &day);
    assert(streak.current_streak == 1);

    day.day = 15;
    rpg_streak_refresh(&streak, &day);
    assert(streak.current_streak == 0);

    rpg_streak_on_quest_complete(&streak, &day);
    assert(streak.current_streak == 1);
}

int main(void)
{
    test_streak();
    return 0;
}
