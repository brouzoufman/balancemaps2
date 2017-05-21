#library "balancemaps2"
#include "zcommon.acs"


#include "constants.h"
#include "commonFuncs.h"
#include "deathtracker.h"
#include "ghostswitch.h"
#include "returnPoints.h"

script "BMaps_Enter" enter
{
    int pln = PlayerNumber();
    BDeath_SetDeaths(pln, 0);
    BReturn_SetupDefaultPoint(pln);
}

script "BMaps_Death" death
{
    int pln = PlayerNumber();
    BDeath_ModDeaths(pln, 1);
    
    int markCount = BDeath_FindSectorMarks(0);
    str markStr   = "";
    
    for (int i = 0; i < markCount; i++)
    {
        if (i > 0) { markStr = StrParam(s:markStr, s:", "); }
        
        markStr = StrParam(s:markStr, s:"<player ", d:BDeath_CheckResult_Player(i), s:" for switch ", d:BDeath_CheckResult_ID(i), s:">");
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
    else
    {
        int gotoID = BReturn_ReturnToPoint(false);
        
        Print(s:"You should be at point ", d:gotoID);
    }
}

script "BMaps_Disconnect" (int pln) disconnect
{    
    BDeath_Disassociate(pln);
    BReturn_UnsetDefaultPoint(pln);
}