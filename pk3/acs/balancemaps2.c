#library "balancemaps2"
#include "zcommon.acs"

#include "commonFuncs.h"

int BMaps_PlayerTIDs[PLAYERMAX];
int BMaps_RanEnter[PLAYERMAX];

#include "constants.h"
#include "deathtracker.h"
#include "ghostswitch.h"
#include "returnPoints.h"

script "BMaps_Enter" enter
{
    int pln = PlayerNumber();
    BMaps_RanEnter[pln] = true;
    
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
    int pln = PlayerNumber();
    if (!BMaps_RanEnter[pln]) { ACS_ExecuteWithResult("BMaps_Enter"); }
    
    BReturn_ReturnToPoint(true);
}

script "BMaps_Disconnect" (int pln) disconnect
{    
    BDeath_Disassociate(pln);
    BReturn_UnsetDefaultPoint(pln);
    BMaps_RanEnter[pln] = false;
}



script "BMaps_Death" death
{
    int pln   = PlayerNumber();
    int myTID = ActivatorTID();
    int killerPln;
    
    // First, check our direct killer
    SetActivatorToTarget(0);
    killerPln = PlayerNumber();
    
    // Might not be a player itself, but something marked by that player
    if (killerPln == -1)
    {
        int markedBy = CheckInventory("MarkedByPlayer") - 1;
        if (markedBy > -1) { killerPln = markedBy; }
    }
    
    // Who cares if killerPln is negative, the script can handle that
    ACS_NamedExecuteWithResult("BMaps_HandleKilledBy", pln, killerPln);
    
    // Then find active sector marks
    int markCount = BDeath_FindSectorMarks(0);
    
    for (int i = 0; i < markCount; i++)
    {
        killerPln = BDeath_CheckResult_Player(i);
        ACS_NamedExecuteWithResult("BMaps_HandleKilledBy", pln, killerPln);
    }
    
    // And finally, dying just because you're bad
    SetActivator(myTID);
    BDeath_ModDeaths(pln, 1);
    
    if (BDeath_GetDeaths(pln) >= BDEATH_MAXDEATHS)
    {
        ACS_NamedExecuteWithResult("BMaps_BecomeGhost", -1);
    }
}


script "BMaps_HandleKilledBy" (int killedPln, int killerPln)
{
    Log(s:"Player ", d:killedPln, s:" killed by player ", d:killerPln);
    if (killedPln < 0 || killerPln < 0 || killerPln == killedPln) { terminate; }
    
    // At this point, the killed player was killed by a different player
    int killedTID = BMaps_PlayerTIDs[killedPln];
    int killerTID = BMaps_PlayerTIDs[killerPln];
    
    // Only people without a body of their own can steal one...
    if (!CheckActorInventory(killerTID, "ShouldBeGhost")) { terminate; }
    
    // ... and only live players can have their body stolen.
    if (CheckActorInventory(killedTID, "GhostSwitchActivator")) { terminate; }
    
    SetActivator(killedTID);
    ACS_NamedExecuteWithResult("BMaps_BecomeGhost", killerPln);
    
    SetActivator(killerTID);
    ACS_NamedExecuteWithResult("BMaps_RewardKill", killedPln);
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
    if (CheckInventory("WillBeGhost") || !CheckInventory("ShouldBeGhost")) { terminate; }
    
    GiveInventory("WillBeGhost", 1);
    Print(s:"You killed ", n:killedPln+1, s:"\c- and claimed their physical form.\n\nNow take their place, and steal their glory.");
    Delay(70);
    
    int pln = PlayerNumber();
    BDeath_SetDeaths(pln, 0);
    TakeInventory("ShouldBeGhost", 0x7FFFFFFF);
    TakeInventory("WillBeGhost",   0x7FFFFFFF);
    
    int theirPoint = BReturn_GetPlayerPoint(killedPln);
    BReturn_SetPlayerPoint(pln, theirPoint);
    BReturn_ReturnToPoint(false);
    
}