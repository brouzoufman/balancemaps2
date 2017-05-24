#library "balancemaps2"
#include "zcommon.acs"

#include "commonFuncs.h"

int BMaps_PlayerTIDs[PLAYERMAX];
int BMaps_TIDUpdater[PLAYERMAX];
int BMaps_RanEnter[PLAYERMAX];
int BMaps_SpawnTic[PLAYERMAX];

#include "constants.h"
#include "deathtracker.h"
#include "deathmarks.h"
#include "ghostswitch.h"
#include "returnPoints.h"

script "BMaps_Open" open
{
    while (true)
    {
        for (int i = 0; i < PLAYERMAX; i++)
        {
            if (PlayerInGame(i)) { BMark_PruneMarks(i); }
        }
        
        Delay(1);
    }
}


script "BMaps_Enter" enter
{
    GiveInventory("NoLongerNoDamage", 1);
    int pln = PlayerNumber();
    ACS_NamedExecuteWithResult("BMaps_UpdatePlayerTID");
    BMaps_RanEnter[pln] = true;
    
    BDeath_SetDeaths(pln, 0);
    BReturn_SetupDefaultPoint(pln);
    
    // In case there's a teleport hook that reacts to point -1
    if (ACS_NamedExecuteWithResult("BReturn_TeleportPointHook", -1) != -1)
    {
        BReturn_ReturnToPoint(true);
    }
    
    if (!isDead(0)) { BMaps_SpawnTic[pln] = Timer() + 1; }
    
    while (true)
    {
        
        if (!isDead(0))
        {
            if (CheckInventory("ShouldBeGhost"))
            {
                if (!CheckActorClass(0, "SpookyGhost"))
                {
                    MorphActor(0, "SpookyGhost", "", 0x7FFFFFFF, MRF_TRANSFERTRANSLATION | MRF_FULLHEALTH, "NoFog", "NoFog");
                    GiveInventory("SpookyGhostMorphPackage", 1);
                }
            }
            else
            {
                if (CheckActorClass(0, "SpookyGhost"))
                {
                    UnmorphActor(0, true);
                    GiveInventory("SpookyGhostUnmorphPackage", 1);
                }
            }
        }
        else
        {
            BMark_ClearMarks(pln);
        }
        
        Delay(1);
    }
}

script "BMaps_Respawn" respawn
{
    GiveInventory("NoLongerNoDamage", 1);
    int pln = PlayerNumber();
    if (!BMaps_RanEnter[pln]) { ACS_NamedExecuteWithResult("BMaps_Enter"); }
    
    if (!isDead(0)) { BMaps_SpawnTic[pln] = Timer() + 1; }
    ACS_NamedExecuteWithResult("BMaps_UpdatePlayerTID");
    BReturn_ReturnToPoint(true);
}

script "BMaps_Disconnect" (int pln) disconnect
{
    BDeath_Disassociate(pln);
    BMark_ClearMarks(pln);
    
    // somehow disconnect can run AFTER enter if a player leaves and rejoins in the same tic
    if (PlayerInGame(pln)) { terminate; }
    
    BReturn_UnsetDefaultPoint(pln);
    BMaps_RanEnter[pln] = false;
    BMaps_SpawnTic[pln] = 0;
}

int BMaps_WhoKilledMe[PLAYERMAX];

script "BMaps_Death" death
{
    // If you get telefragged, death scripts can run before enter/respawn scripts.
    //  This is retarded, but we *need* player TIDs to be set properly.
    ACS_NamedExecuteWithResult("BMaps_UpdatePlayerTID");
    
    int pln   = PlayerNumber();
    int myTID = ActivatorTID();
    int killerPln = -1;
    int i;
    
    // This is specifically so instant deaths (aka telefrags) don't count.
    int spawnTic = BMaps_SpawnTic[pln] - 1;
    Log(s:"spawnTic for player ", d:pln, s:": ", d:spawnTic);
    if (spawnTic < 0 || spawnTic >= Timer()) { terminate; }
    
    // Now that we're dead, make future deaths until next respawn script run ineligible.
    // There is a difference between "next respawn" and "next respawn script run",
    // and there really shouldn't be.
    BMaps_SpawnTic[pln] = 0;
    
    for (i = 0; i < PLAYERMAX; i++) { BMaps_WhoKilledMe[i] = false; }
    
    // First, check our direct killer
    if (SetActivatorToTarget(0))
    {
        killerPln = PlayerNumber();
        
        // Might not be a player itself, but something marked by that player
        if (killerPln == -1)
        {
            int markedBy = CheckInventory("MarkedByPlayer") - 1;
            if (markedBy > -1) { killerPln = markedBy; }
        }
        else
        {
            // I'm slapping this stupid fucking thing everywhere
            //  because for some reason telefragging happens before
            //  any scripts can run
            ACS_NamedExecuteWithResult("BMaps_UpdatePlayerTID");
        }
        
        if (killerPln >= 0) { BMaps_WhoKilledMe[killerPln] = true; }
    }
    
    SetActivator(myTID);
    
    // Now check who marked us for death
    int markCount = BMark_PlayerMarkCount[pln];
    for (i = 0; i < markCount; i++)
    {
        killerPln = BMark_PlayerMarks[pln][i][0];
        if (killerPln >= 0) { BMaps_WhoKilledMe[killerPln] = true; }
    }

    BMark_ClearMarks(pln);
    
    
    // Then find active sector marks
    markCount = BDeath_FindSectorMarks(0);
    for (i = 0; i < markCount; i++)
    {
        killerPln = BDeath_CheckResult_Player(i);
        if (killerPln >= 0) { BMaps_WhoKilledMe[killerPln] = true; }
    }
    
    
    // Now credit those assholes.
    for (i = 0; i < PLAYERMAX; i++)
    {
        if (BMaps_WhoKilledMe[i])
        {
            if (i == pln) { Print(s:"You killed yourself you idiot"); }
            ACS_NamedExecuteWithResult("BMaps_HandleKilledBy", pln, i);
        }
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
    
    // I really shouldn't need this
    if (killedTID != 0)
    {
        SetActivator(killedTID);
        ACS_NamedExecuteWithResult("BMaps_BecomeGhost", killerPln);
    }
    
    if (killerTID != 0)
    {
        SetActivator(killerTID);
        ACS_NamedExecuteWithResult("BMaps_RewardKill", killedPln);
    }
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
        Print(s:"Your physical form has been stolen.\n\nSend someone to their doom, and mete your revenge.");
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


script "BMaps_UpdatePlayerTID" (void)
{
    int pln    = PlayerNumber();
    int myLock = ++BMaps_TIDUpdater[pln];
    
    while (BMaps_TIDUpdater[pln] == myLock)
    {
        BMaps_PlayerTIDs[pln] = defaultTID(-1);
        Delay(1);
    }
}

script "BMaps_GetPlayerTID" (int pln)
{
    if (PlayerInGame(pln)) { SetResultValue(BMaps_PlayerTIDs[pln]); }
    else { SetResultValue(-1); }
}