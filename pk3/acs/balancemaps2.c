#library "balancemaps2"
#include "zcommon.acs"

#include "commonFuncs.h"
#include "version.h"

int BMaps_PlayerTIDs[PLAYERMAX];
int BMaps_TIDUpdater[PLAYERMAX];
int BMaps_RanEnter[PLAYERMAX];
int BMaps_SpawnTic[PLAYERMAX];
int BMaps_FinishTimes[PLAYERMAX];

int IsServer;

function int IsZand(void)
{
    SetDBEntry("balancemaps", "iszand", 18271);
    return GetDBEntry("balancemaps", "iszand") == 18271;
}

#include "constants.h"
#include "deathtracker.h"
#include "deathmarks.h"
#include "gamestate.h"
#include "ghostswitch.h"
#include "mapscripts.h"
#include "returnPoints.h"
#include "timer.h"
#include "gotoplayer.h"

#include "decorate.h"

script "BMaps_Open" open
{
    IsServer = true;
    
    if (IsZand())
    {
		if (!GetCVar("sv_unblockplayers")) { ConsoleCommand("sv_unblockplayers 1"); }
		if (!GetCVar("sv_norespawninvul")) { ConsoleCommand("sv_norespawninvul 1"); }
		if (!GetCVar("sv_forcerespawn"))
        {
			ConsoleCommand("sv_forcerespawn 1");
			ConsoleCommand("alwaysapplydmflags 1");
		}
        
        if (!(GetCVar("survival") || GetCVar("bmaps_debug_notsurvival")))
        {
            if (ConsolePlayerNumber() == -1)
            {
                Log(s:"For future reference, the game type must be survival for balancemaps to work");
            }
            
            HudMessage(s:"Game type isn't survival. Restarting map...";
                HUDMSG_PLAIN, 1000, CR_WHITE, 1.5, 0.75, 5.0);
            
            Delay(105);
            ConsoleCommand("survival 1");
            ConsoleCommand("sv_maxlives 999");
            ConsoleCommand(StrParam(s:"map ", n:PRINTNAME_LEVEL));
        }
    }
}


script "BMaps_Enter" enter
{
    int pln = PlayerNumber();
    if (BMaps_RanEnter[pln]) { terminate; }
    
    GiveInventory("NoLongerNoDamage", 1);
    
    if (CheckInventory("ShouldBeGhost"))
    {
        UnmorphActor(0, true);
        GiveInventory("SpookyGhostUnmorphPackage", 1);
        TakeInventory("ShouldBeGhost", 0x7FFFFFFF);
    }
    
    ACS_NamedExecuteWithResult("BMaps_UpdatePlayerTID");
    BMaps_RanEnter[pln] = true;
    
    BDeath_SetDeaths(pln, 0);
    BReturn_SetupDefaultPoint(pln);
    
    BReturn_ReturnToPoint(true, true);
    
    if (!isDead(0)) { BMaps_SpawnTic[pln] = Timer() + 1; }
    
    while (true)
    {
        int dead               = isDead(0);
        int livesLeft_survival = GetPlayerLivesLeft(pln);
        int livesLeft_core     = BDeath_LivesLeft(pln);
        
        if (dead) { BMark_ClearMarks(pln); }
        
        BState_CheckGameState();
        
        if (BState_GameLost)
        {
            if (!dead) { GiveInventory("PleaseJustFuckingDie", 1); }
            SetPlayerLivesLeft(pln, 0);
            Delay(1);
            continue;
        }
        
        if (!dead)
        {
            if (livesLeft_survival != livesLeft_core)
            {
                SetPlayerLivesLeft(pln, livesLeft_core);
            }
            
            if (CheckInventory("ShouldBeGhost"))
            {
                if (!dead && !CheckActorClass(0, "SpookyGhost"))
                {
                    MorphActor(0, "SpookyGhost", "", 0x7FFFFFFF, MRF_FULLHEALTH | MRF_WHENINVULNERABLE, "NoFog", "NoFog");
                    GiveInventory("SpookyGhostMorphPackage", 1);
                }
            }
            else
            {
                if (!dead && CheckActorClass(0, "SpookyGhost"))
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
    GiveInventory("NoLongerNoDamage", 1);
    int pln = PlayerNumber();
    if (!BMaps_RanEnter[pln]) { ACS_NamedExecuteWithResult("BMaps_Enter"); }
    
    if (!isDead(0)) { BMaps_SpawnTic[pln] = Timer() + 1; }
    ACS_NamedExecuteWithResult("BMaps_UpdatePlayerTID");
    BReturn_ReturnToPoint(true, false);
}


script "BMaps_Disconnect" (int pln) disconnect
{
    BDeath_Disassociate(pln);
    BMark_ClearMarks(pln);
    
    // somehow disconnect can run AFTER enter if a player leaves and rejoins in the same tic
    if (PlayerInGame(pln)) { terminate; }
    
    BReturn_UnsetDefaultPoint(pln);
    BMaps_RanEnter[pln]   = false;
    BMaps_SpawnTic[pln]   = 0;
    BMaps_PlayerTIDs[pln] = -1;
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
    
    if (BDeath_LivesLeft(pln) <= 0)
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
    
    // This'll handle if the killer left
    if (killedTID == -1 || killerTID == -1) { terminate; }
    
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
    if (CheckInventory("WillBeHuman") || !CheckInventory("ShouldBeGhost")) { terminate; }
    
    GiveInventory("WillBeHuman", 1);
    Print(s:"You killed ", n:killedPln+1, s:"\c- and claimed their physical form.\n\nNow take their place, and steal their glory.");
    Delay(70);
    
    int pln = PlayerNumber();
    BDeath_SetDeaths(pln, 0);
    TakeInventory("ShouldBeGhost", 0x7FFFFFFF);
    TakeInventory("WillBeHuman",   0x7FFFFFFF);
    
    int theirPoint = BReturn_GetPlayerPoint(killedPln);
    BReturn_SetPlayerPoint(pln, theirPoint);
    BReturn_ReturnToPoint(false, false);
    
}


script "BMaps_UpdatePlayerTID" (void)
{
    int pln    = PlayerNumber();
    int myLock = ++BMaps_TIDUpdater[pln];
    
    while (PlayerInGame(pln) && BMaps_TIDUpdater[pln] == myLock)
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