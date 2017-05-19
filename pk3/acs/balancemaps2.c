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
    
    int markCount = BDeath_FindSectorMarks(0);
    str markStr   = "";
    
    for (int i = 0; i < markCount; i++)
    {
        if (i > 0) { markStr = StrParam(s:markStr, s:", ", d:BDeath_MarkedByResult(i)); }
        else       { markStr = StrParam(s:markStr,         d:BDeath_MarkedByResult(i)); }
    }
    
    Log(s:"Sector marks: ", s:markStr);
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