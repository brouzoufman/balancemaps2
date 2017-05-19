#define BDEATH_MAXDEATHS            2
#define BDEATH_MAXMARKEDSECTORS     256

int BDeath_AccumulatedDeaths[PLAYERMAX];

int BDeath_MarkedSectors[BDEATH_MAXMARKEDSECTORS][2];
int BDeath_MarkCount = 0;