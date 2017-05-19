#library "balancemaps2"
#include "zcommon.acs"

#include "commonFuncs.h"
#include "deathtracker.h"
#include "ghostswitch.h"

script "BMaps_Enter" enter
{
    int pln = PlayerNumber();
    BDeath_SetDeaths(pln, 0);
}

script "BMaps_Death" death
{
    int pln = PlayerNumber();
    BDeath_ModDeaths(pln, 1);
    
    int checkTID = UniqueTID();
    
    SpawnForced("SectorMarkCheck", GetActorX(0), GetActorY(0), GetActorZ(0), checkTID);
    Print(s:"Sector mark: ", d:BDeath_FindSectorMark(checkTID));
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