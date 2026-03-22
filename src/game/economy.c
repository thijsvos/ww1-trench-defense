#include "economy.h"

void economy_init(Economy *eco, int starting_gold, int starting_lives)
{
    eco->gold             = starting_gold;
    eco->lives            = starting_lives;
    eco->score            = 0;
    eco->total_kills      = 0;
    eco->total_gold_earned = 0;
}

bool economy_can_afford(Economy *eco, int cost)
{
    return eco->gold >= cost;
}

void economy_spend(Economy *eco, int cost)
{
    if (economy_can_afford(eco, cost)) {
        eco->gold -= cost;
    }
}

void economy_earn(Economy *eco, int amount)
{
    eco->gold             += amount;
    eco->total_gold_earned += amount;
    eco->score            += amount;
}

void economy_lose_lives(Economy *eco, int amount)
{
    eco->lives -= amount;
    if (eco->lives < 0) eco->lives = 0;
}

void economy_add_kill(Economy *eco)
{
    eco->total_kills++;
}

int economy_sell_value(int total_invested)
{
    return (total_invested * 60) / 100;  /* 60% return */
}

bool economy_is_defeated(Economy *eco)
{
    return eco->lives <= 0;
}
