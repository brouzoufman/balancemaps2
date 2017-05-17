#include "defs_deathtracker.h"

function int BDeath_GetDeaths(int pln)
{
    return BDeath_AccumulatedDeaths[pln];
}

function void BDeath_SetDeaths(int pln, int i)
{
    BDeath_AccumulatedDeaths[pln] = i;
}

function void BDeath_ModDeaths(int pln, int i)
{
    BDeath_AccumulatedDeaths[pln] += i;
}