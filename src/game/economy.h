#ifndef TD_ECONOMY_H
#define TD_ECONOMY_H

#include <stdbool.h>

typedef struct Economy {
    int gold;
    int lives;
    int score;
    int total_kills;
    int total_gold_earned;
} Economy;

void economy_init(Economy *eco, int starting_gold, int starting_lives);
bool economy_can_afford(Economy *eco, int cost);
void economy_spend(Economy *eco, int cost);
void economy_earn(Economy *eco, int amount);
void economy_lose_lives(Economy *eco, int amount);
void economy_add_kill(Economy *eco);
int economy_sell_value(int total_invested);  /* returns 60% of invested */
bool economy_is_defeated(Economy *eco);      /* lives <= 0 */

#endif /* TD_ECONOMY_H */
