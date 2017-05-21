#library "balancemaps2"
#include "zcommon.acs"

#include "commonFuncs.h"

int BMaps_PlayerTIDs[PLAYERMAX];

#include "constants.h"
#include "deathtracker.h"
#include "ghostswitch.h"
#include "returnPoints.h"

script "BMaps_Enter" enter
{
    ACS_NamedExecuteWithResult("BMaps_Spawn", true);
}

script "BMaps_Respawn" respawn
{
    ACS_NamedExecuteWithResult("BMaps_Spawn", false);
}

script "BMaps_Spawn" (int entering)
{
    int pln = PlayerNumber();
    
    if (entering)
    {
        BDeath_SetDeaths(pln, 0);
        BReturn_SetupDefaultPoint(pln);
        
        while (true)
        {
            BMaps_PlayerTIDs[pln] = defaultTID(-1);
            Delay(1);
        }
    }
    else
    {
        int curDeaths = BDeath_GetDeaths(pln);
        if (curDeaths >= BDEATH_MAXDEATHS)
        {
            MorphActor(0, "SpookyGhost", "", 0x7FFFFFFF, MRF_TRANSFERTRANSLATION | MRF_FULLHEALTH, "NoFog", "NoFog");
            GiveInventory("SpookyGhostMorphPackage", 1);
        }
    }
    
    BReturn_ReturnToPoint(true);
}



script "BMaps_Death" death
{
    int pln = PlayerNumber();
    BDeath_ModDeaths(pln, 1);
    
    int markCount = BDeath_FindSectorMarks(0);
    str markStr   = "";
    
    for (int i = 0; i < markCount; i++)
    {
        int killerPln = BDeath_CheckResult_Player(i);
        
        if (pln == killerPln)
        {
            Print(s:"You killed yourself you idiot");
        }
        else if (killerPln >= 0)
        {
            SetActivator(BMaps_PlayerTIDs[killerPln]);
            Print(s:"You killed ", n:pln+1);
        }
    }
    
    SetActivatorToPlayer(pln);
}

script "BMaps_Disconnect" (int pln) disconnect
{    
    BDeath_Disassociate(pln);
    BReturn_UnsetDefaultPoint(pln);
}