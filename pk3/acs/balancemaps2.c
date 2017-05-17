#library "balancemaps2"
#include "zcommon.acs"

#include "funcs_commonFuncs.h"
#include "funcs_deathtracker.h"

script "BMaps_Enter" enter
{
    int pln = PlayerNumber();
    BDeath_SetDeaths(pln, 0);
}

script "BMaps_Death" death
{
    int pln = PlayerNumber();
    BDeath_ModDeaths(pln, 1);
}

script "BMaps_Respawn" respawn
{
    int pln       = PlayerNumber();
    int curDeaths = BDeath_GetDeaths(pln);
    
    if (curDeaths >= BDEATH_MAXDEATHS)
    {
        Print(s:"You should be a spooky ghost right now");
    }
}