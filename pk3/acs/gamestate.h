int BState_LastGameStateCheck = -1;
int BState_GameLost;

function void BState_CheckGameState(void)
{
    if (BState_LastGameStateCheck >= Timer()) { return; }
    BState_LastGameStateCheck = Timer();
    
    if  (GetCVar("bmaps_debug_neverlose")) { BState_GameLost = false; return; }
    
    int playersAlive = 0;
        
    for (int i = 0; i < PLAYERMAX; i++)
    {
        if (!PlayerInGame(i)) { continue; }
        BMark_PruneMarks(i);
        
        // bots are retarded and can't do maps, so just assume they're dead
        //  even if they aren't they might as well be
        //
        // this is mainly to protect against idiot hosts who add bots,
        //  clogging up the game for as long as it takes for the bots to
        //  commit suicide by running off a cliff or something
        if (PlayerIsBot(i) && !GetCVar("bmaps_debug_botscount")) { continue; }
        
        int playerTID = BMaps_PlayerTIDs[i];
        if (playerTID == -1)
        {
            playersAlive += 1; // he probably spawned this tic and
            continue;          //  his enter script hasn't run yet
        }
        
        if (!CheckActorInventory(playerTID, "ShouldBeGhost")
          || CheckActorInventory(playerTID, "WillBeHuman"))   { playersAlive += 1; }
    }
    
    BState_GameLost = (PlayerCount() > 0) && (playersAlive == 0);
}