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
    int pln = PlayerNumber();
    
    BDeath_SetDeaths(pln, 0);
    BReturn_SetupDefaultPoint(pln);
    
    // This is mainly in case there's a teleport hook
    BReturn_ReturnToPoint(true);
    
    while (true)
    {
        BMaps_PlayerTIDs[pln] = defaultTID(-1);
        
        if (!isDead(0))
        {
            if (CheckInventory("ShouldBeGhost"))
            {
                if (!CheckInventory("GhostSwitchActivator"))
                {
                    MorphActor(0, "SpookyGhost", "", 0x7FFFFFFF, MRF_TRANSFERTRANSLATION | MRF_FULLHEALTH, "NoFog", "NoFog");
                    GiveInventory("SpookyGhostMorphPackage", 1);
                }
            }
            else
            {
                if (CheckInventory("GhostSwitchActivator"))
                {
                    UnmorphActor(0, true);
                    GiveInventory("SpookyGhostUnmorphPackage", 1);
                }
            }
        }
        
        Delay(1);
    }
}

script "BMaps_Respawn" respawn
{
    BReturn_ReturnToPoint(true);
}



script "BMaps_Death" death
{
    int pln   = PlayerNumber();
    int myTID = ActivatorTID();
    
    int markCount = BDeath_FindSectorMarks(0);
    str markStr   = "";
    
    int killerPln, killerTID;
    
    for (int i = 0; i < markCount; i++)
    {
        killerPln = BDeath_CheckResult_Player(i);
        
        if (pln == killerPln)
        {
            Print(s:"You killed yourself you idiot");
        }
        else if (killerPln >= 0)
        {
            killerTID = BMaps_PlayerTIDs[killerPln];
            
            if (CheckActorInventory(killerTID, "ShouldBeGhost"))
            {
                ACS_NamedExecuteWithResult("BMaps_BecomeGhost", killerPln);
                SetActivator(killerTID);
                ACS_NamedExecuteWithResult("BMaps_RewardKill", pln);
                SetActivator(myTID);
            }
        }
    }
    
    // A direct kill should never happen, but if it does, reward that too
    SetActivatorToTarget(0);
    killerPln = PlayerNumber();
    killerTID = ActivatorTID();
    
    if (killerPln > -1 && pln != killerPln)
    {
        if (CheckInventory("ShouldBeGhost"))
        {
            ACS_NamedExecuteWithResult("BMaps_RewardKill", pln);
            SetActivator(myTID);
            ACS_NamedExecuteWithResult("BMaps_BecomeGhost", killerPln);
        }
    }
    
    SetActivator(myTID);
    BDeath_ModDeaths(pln, 1);
    
    if (BDeath_GetDeaths(pln) >= BDEATH_MAXDEATHS)
    {
        ACS_NamedExecuteWithResult("BMaps_BecomeGhost", -1);
    }
}

script "BMaps_Disconnect" (int pln) disconnect
{    
    BDeath_Disassociate(pln);
    BReturn_UnsetDefaultPoint(pln);
}


script "BMaps_BecomeGhost" (int killerPln)
{
    if (CheckInventory("ShouldBeGhost")) { terminate; }
    GiveInventory("ShouldBeGhost", 1);
    
    if (killerPln == -1)
    {
        Print(s:"Your spirit has been disembodied.\n\nSend someone to their doom, and reclaim your form.");
    }
    else
    {
        Print(n:killerPln+1, s:"\c- has claimed your physical form.\n\nSend someone to their doom, and mete your revenge.");
    }
        
}
    

script "BMaps_RewardKill" (int killedPln)
{
    if (!CheckInventory("ShouldBeGhost")) { terminate; }
    
    Print(s:"You killed ", n:killedPln+1, s:"\c- and claimed their physical form.\n\nNow take their place, and steal their glory.");
    Delay(70);
    
    int pln = PlayerNumber();
    BDeath_SetDeaths(pln, 0);
    TakeInventory("ShouldBeGhost", 1);
    
    int theirPoint = BReturn_GetPlayerPoint(killedPln);
    BReturn_SetPlayerPoint(pln, theirPoint);
    BReturn_ReturnToPoint(false);
    
}