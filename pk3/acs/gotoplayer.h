str GotoCamTextures[PLAYERMAX] =
{
    "TPVIEW00", "TPVIEW01", "TPVIEW02", "TPVIEW03", "TPVIEW04", "TPVIEW05", "TPVIEW06", "TPVIEW07", 
    "TPVIEW08", "TPVIEW09", "TPVIEW10", "TPVIEW11", "TPVIEW12", "TPVIEW13", "TPVIEW14", "TPVIEW15", 
    "TPVIEW16", "TPVIEW17", "TPVIEW18", "TPVIEW19", "TPVIEW20", "TPVIEW21", "TPVIEW22", "TPVIEW23", 
    "TPVIEW24", "TPVIEW25", "TPVIEW26", "TPVIEW27", "TPVIEW28", "TPVIEW29", "TPVIEW30", "TPVIEW31", 
    "TPVIEW32", "TPVIEW33", "TPVIEW34", "TPVIEW35", "TPVIEW36", "TPVIEW37", "TPVIEW38", "TPVIEW39", 
    "TPVIEW40", "TPVIEW41", "TPVIEW42", "TPVIEW43", "TPVIEW44", "TPVIEW45", "TPVIEW46", "TPVIEW47", 
    "TPVIEW48", "TPVIEW49", "TPVIEW50", "TPVIEW51", "TPVIEW52", "TPVIEW53", "TPVIEW54", "TPVIEW55", 
    "TPVIEW56", "TPVIEW57", "TPVIEW58", "TPVIEW59", "TPVIEW60", "TPVIEW61", "TPVIEW62", "TPVIEW63",
};

function int MoveAlongLine_KeepSight(int camTID, int sightTID, int startX, int startY, int startZ, int dx, int dy, int dz, int endShift)
{
    int mag = VectorLength(VectorLength(dx, dy), dz);
    int nx  = FixedDiv(dx, mag);
    int ny  = FixedDiv(dy, mag);
    int nz  = FixedDiv(dz, mag);
    
    int testMag     = mag >> 1;
    int shiftFactor = 2;
    
    int lastX, lastY, lastZ;
    int wasVisible = false;
    
    while (true)
    {
        int testX = startX + FixedMul(nx, testMag);
        int testY = startY + FixedMul(ny, testMag);
        int testZ = startZ + FixedMul(nz, testMag);
        
        int warped = Warp(camTID, testX, testY, testZ, 0, WARPF_MOVEPTR | WARPF_ABSOLUTEPOSITION | WARPF_COPYINTERPOLATION);
        
        if (warped && CheckSight(camTID, sightTID, 0))
        {
            testMag += (mag >> shiftFactor);
            wasVisible = true;
            
            lastX = testX;
            lastY = testY;
            lastZ = testZ;
        }
        else
        { 
            testMag -= (mag >> shiftFactor);
            
            if (wasVisible)
            {
                Warp(camTID, lastX, lastY, lastZ, 0, WARPF_MOVEPTR | WARPF_ABSOLUTEPOSITION | WARPF_COPYINTERPOLATION);
            }    
        }
        
        shiftFactor += 1;
        
        if ((mag >> shiftFactor) == 0) { break; }
    }
     
    // hack: since Warp doesn't care about radius when checking collisions with
    //  the lower/upper parts of sectors, we use endShift
    if (endShift)
    {
        testMag = max(0, testMag + endShift);
        
        Warp(camTID, startX + FixedMul(nx, testMag), startY + FixedMul(ny, testMag), startZ + FixedMul(nz, testMag),
                     0, WARPF_MOVEPTR | WARPF_NOCHECKPOSITION | WARPF_ABSOLUTEPOSITION | WARPF_COPYINTERPOLATION);
    }
    
    return testMag;
}

script "BGoto_WarpCamera" (int plnToView) clientside
{
    if (!PlayerInGame(plnToView)) { terminate; }
    SetActivatorToPlayer(plnToView);
    
    int camTID = UniqueTID();
    int camSpawned = SpawnForced("GotoPlayer_Camera", GetActorX(0), GetActorY(0), GetActorZ(0), camTID);
    if (!camSpawned) { terminate; }
    
    SetResultValue(camTID);
    
    while (IsTIDUsed(camTID))
    {
        if (!PlayerInGame(plnToView)) { break; }
        
        // plan here: start at player's head, put the camera at the desired position,
        // and if the camera can't see his head, move it closer or further (binary chop)
        // until the optimal position is found
        //
        // player's view height is 41 units by default, so +9 puts it at 50 units,
        // just about where his eyes actually are on the sprite
        
        int headHeight = GetActorViewHeight(0);
        
        int headTID = UniqueTID();
        int headX   = GetActorX(0);
        int headY   = GetActorY(0);
        int headZ   = GetActorZ(0) + headHeight;
        
        SpawnForced("GotoPlayer_SightChecker", headX, headY, headZ, headTID);
        int warped = Warp(camTID, -96.0, 0, headHeight, 0, WARPF_MOVEPTR | WARPF_NOCHECKPOSITION | WARPF_COPYINTERPOLATION);
        
        if (!(warped && CheckSight(camTID, headTID, 0)))
        {
            int dx  = GetActorX(camTID) - headX;
            int dy  = GetActorY(camTID) - headY;
            int dz  = GetActorZ(camTID) - headZ;
            
            MoveAlongLine_KeepSight(camTID, headTID, headX,headY,headZ, dx,dy,dz, -1.0);
        }
        
        SetActorPosition(camTID, GetActorX(camTID), GetActorY(camTID), min(GetActorZ(camTID) + 16.0, GetActorCeilingZ(camTID)), false);
        
        Thing_Remove(headTID);
        Delay(1);
    }
    
    if (IsTIDUsed(camTID)) { Thing_Remove(camTID); }
}

script "BGoto_DebugCamera" (void) clientside
{
    int pln        = PlayerNumber();
    int camTID     = ACS_NamedExecuteWithResult("BGoto_WarpCamera", pln);
    str camTexName = GotoCamTextures[pln];
    
    if (camTID)
    {
        SetCameraToTexture(camTID, camTexName, 90);
        
        SetHudSize(800, 600, true);
        SetFont(camTexName);
        HudMessage(s:"A"; HUDMSG_PLAIN, 19828, CR_UNTRANSLATED, 600.4, 300.0, 0);
    }
}