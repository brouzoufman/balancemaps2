int BMaps_LastGameStateCheck = -1;
int BMaps_GameLost;

function void BMaps_CheckGameState(void)
{
    if (BMaps_LastGameStateCheck >= Timer()) { return; }
    BMaps_LastGameStateCheck = Timer();
    
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
        if (PlayerIsBot(i)) { continue; }
        
        int playerTID = BMaps_PlayerTIDs[i];
        if (playerTID == -1)
        {
            playersAlive += 1; // he probably spawned this tic and
            continue;          //  his enter script hasn't run yet
        }
        
        if (!CheckActorInventory(playerTID, "ShouldBeGhost")) { playersAlive += 1; }
    }
    
    BMaps_GameLost = (PlayerCount() > 0) && (playersAlive == 0);
}