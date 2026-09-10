#include "rpg_date.h"

static rpg_date_provider_t s_provider;

void rpg_date_set_provider(rpg_date_provider_t provider)
{
    s_provider = provider;
}

void rpg_date_today(rpg_date_t *out)
{
    if (!out) return;

    if (s_provider) {
        s_provider(out);
        return;
    }

    out->year = 2026;
    out->month = 1;
    out->day = 1;
}

static int32_t days_from_civil(int year, unsigned month, unsigned day)
{
    year -= month <= 2;
    const int era = (year >= 0 ? year : year - 399) / 400;
    const unsigned yoe = (unsigned)(year - era * 400);
    const unsigned doy = (153u * (month + (month > 2 ? -3 : 9)) + 2u) / 5u + day - 1u;
    const unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
    return (int32_t)(era * 146097 + (int)doe - 719468);
}

uint32_t rpg_date_to_serial(const rpg_date_t *date)
{
    if (!date || date->month < 1 || date->month > 12 || date->day < 1 || date->day > 31) {
        return 0;
    }

    int32_t serial = days_from_civil(date->year, date->month, date->day);
    return serial < 0 ? 0 : (uint32_t)serial;
}
