#define CAMTEX_NORMAL   0
#define CAMTEX_SMALL    1

str GotoCamTextures[2][PLAYERMAX] =
{
    {
        "TPVIEW00", "TPVIEW01", "TPVIEW02", "TPVIEW03", "TPVIEW04", "TPVIEW05", "TPVIEW06", "TPVIEW07", 
        "TPVIEW08", "TPVIEW09", "TPVIEW10", "TPVIEW11", "TPVIEW12", "TPVIEW13", "TPVIEW14", "TPVIEW15", 
        "TPVIEW16", "TPVIEW17", "TPVIEW18", "TPVIEW19", "TPVIEW20", "TPVIEW21", "TPVIEW22", "TPVIEW23", 
        "TPVIEW24", "TPVIEW25", "TPVIEW26", "TPVIEW27", "TPVIEW28", "TPVIEW29", "TPVIEW30", "TPVIEW31", 
        "TPVIEW32", "TPVIEW33", "TPVIEW34", "TPVIEW35", "TPVIEW36", "TPVIEW37", "TPVIEW38", "TPVIEW39", 
        "TPVIEW40", "TPVIEW41", "TPVIEW42", "TPVIEW43", "TPVIEW44", "TPVIEW45", "TPVIEW46", "TPVIEW47", 
        "TPVIEW48", "TPVIEW49", "TPVIEW50", "TPVIEW51", "TPVIEW52", "TPVIEW53", "TPVIEW54", "TPVIEW55", 
        "TPVIEW56", "TPVIEW57", "TPVIEW58", "TPVIEW59", "TPVIEW60", "TPVIEW61", "TPVIEW62", "TPVIEW63",
    },
    {
        "TPTINY00", "TPTINY01", "TPTINY02", "TPTINY03", "TPTINY04", "TPTINY05", "TPTINY06", "TPTINY07", 
        "TPTINY08", "TPTINY09", "TPTINY10", "TPTINY11", "TPTINY12", "TPTINY13", "TPTINY14", "TPTINY15", 
        "TPTINY16", "TPTINY17", "TPTINY18", "TPTINY19", "TPTINY20", "TPTINY21", "TPTINY22", "TPTINY23", 
        "TPTINY24", "TPTINY25", "TPTINY26", "TPTINY27", "TPTINY28", "TPTINY29", "TPTINY30", "TPTINY31", 
        "TPTINY32", "TPTINY33", "TPTINY34", "TPTINY35", "TPTINY36", "TPTINY37", "TPTINY38", "TPTINY39", 
        "TPTINY40", "TPTINY41", "TPTINY42", "TPTINY43", "TPTINY44", "TPTINY45", "TPTINY46", "TPTINY47", 
        "TPTINY48", "TPTINY49", "TPTINY50", "TPTINY51", "TPTINY52", "TPTINY53", "TPTINY54", "TPTINY55", 
        "TPTINY56", "TPTINY57", "TPTINY58", "TPTINY59", "TPTINY60", "TPTINY61", "TPTINY62", "TPTINY63",
    },
};



function int MoveAlongLine_KeepSight(int camTID, int sightTID, int startX, int startY, int startZ, int dx, int dy, int dz, int endShift)
{
    int mag = VectorLength(VectorLength(dx, dy), dz);
    int nx  = FixedDiv(dx, mag);
    int ny  = FixedDiv(dy, mag);
    int nz  = FixedDiv(dz, mag);
    
    int testMag     = mag >> 1;
    int shiftFactor = 2;
    
    int camAngle = GetActorAngle(camTID);
    int lastX, lastY, lastZ;
    int wasVisible = false;
    
    while (true)
    {
        int testX = startX + FixedMul(nx, testMag);
        int testY = startY + FixedMul(ny, testMag);
        int testZ = startZ + FixedMul(nz, testMag);
        
        int warped = Warp(camTID, testX, testY, testZ, camAngle, WARPF_MOVEPTR | WARPF_ABSOLUTEPOSITION | WARPF_ABSOLUTEANGLE | WARPF_COPYINTERPOLATION);
        
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
                Warp(camTID, lastX, lastY, lastZ, camAngle, WARPF_MOVEPTR | WARPF_ABSOLUTEPOSITION | WARPF_ABSOLUTEANGLE | WARPF_COPYINTERPOLATION);
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
                     camAngle, WARPF_MOVEPTR | WARPF_NOCHECKPOSITION | WARPF_ABSOLUTEPOSITION | WARPF_ABSOLUTEANGLE | WARPF_COPYINTERPOLATION);
    }
    
    return testMag;
}




script "BGoto_CanWarpTo" (int pln)
{
    if (!PlayerInGame(pln)) { SetResultValue(false); terminate; }
    ActivatorToPlayer(pln);
    
    SetResultValue(!CheckActorClass(0, "SpookyGhost"));
}



script "BGoto_GetNearestPlayer" (int noghost)
{
    int myX      = GetActorX(0);
    int myY      = GetActorY(0);
    int myZ      = GetActorZ(0);
    int myRadius = GetActorProperty(0, APROP_Radius);
    int myHeight = GetActorProperty(0, APROP_Height);
    int myPln    = PlayerNumber();
    
    int nearestPln  = -1;
    int nearestDist = 0x7FFFFFFF;
    
    for (int i = 0; i < PLAYERMAX; i++)
    {
        if (i == myPln || !PlayerInGame(i)) { continue; }
        
        ActivatorToPlayer(i);
        if (noghost && CheckActorClass(0, "SpookyGhost")) { continue; }
        
        int plX      = GetActorX(0);
        int plY      = GetActorY(0);
        int plZ      = GetActorZ(0);
        int plRadius = GetActorProperty(0, APROP_Radius);
        int plHeight = GetActorProperty(0, APROP_Height);
        
        int myNearestX = middle(myX - myRadius, plX, myX + myRadius);
        int myNearestY = middle(myY - myRadius, plY, myY + myRadius);
        int myNearestZ = middle(myZ,            plZ, myZ + myHeight);
        
        int plNearestX = middle(plX - plRadius, myX, plX + myRadius);
        int plNearestY = middle(plY - plRadius, myY, plY + myRadius);
        int plNearestZ = middle(plZ,            myZ, plZ + myHeight);
        
        int dx = myNearestX - plNearestX;
        int dy = myNearestY - plNearestY;
        int dz = myNearestZ - plNearestZ;
        
        int dist = VectorLength(VectorLength(dx, dy), dz);
        
        if (dist < NearestDist)
        {
            nearestPln  = i;
            nearestDist = dist;
        }
    }
    
    SetResultValue(nearestPln);
}



script "BGoto_WarpCamera" (int plnToView) clientside
{
    if (!PlayerInGame(plnToView)) { terminate; }
    SetActivatorToPlayer(plnToView);
    
    int camTID = UniqueTID();
    int camSpawned = SpawnForced("GotoPlayer_Camera", GetActorX(0), GetActorY(0), GetActorZ(0), camTID);
    if (!camSpawned) { terminate; }
    
    SetResultValue(camTID);
    
    // when locked, it stays directly behind its target
    // when unlocked, it can rotate on its own
    SetUserVariable(camTID, "user_unlocked", false);
    
    while (IsTIDUsed(camTID))
    {
        if (!PlayerInGame(plnToView)) { break; }
        
        int headHeight = GetActorViewHeight(0);
        
        int headTID = UniqueTID();
        int headX   = GetActorX(0);
        int headY   = GetActorY(0);
        int headZ   = GetActorZ(0) + headHeight;
        
        int camAngle;
        
        if (GetUserVariable(camTID, "user_unlocked"))
        {
            camAngle = GetUserVariable(camTID, "user_angle");
        }
        else
        {
            camAngle = GetActorAngle(0);
            SetUserVariable(camTID, "user_angle", camAngle);
        }
        
        int camX = headX + (96 * cos(camAngle + 0.5));
        int camY = headY + (96 * sin(camAngle + 0.5));
        int camZ = headZ;
        
        SpawnForced("GotoPlayer_SightChecker", headX, headY, headZ, headTID);
        int warped = Warp(camTID, camX, camY, camZ, camAngle, WARPF_MOVEPTR | WARPF_ABSOLUTEPOSITION | WARPF_ABSOLUTEANGLE | WARPF_NOCHECKPOSITION | WARPF_COPYINTERPOLATION);
        
        if (!(warped && CheckSight(camTID, headTID, 0)))
        {
            int dx  = GetActorX(camTID) - headX;
            int dy  = GetActorY(camTID) - headY;
            int dz  = GetActorZ(camTID) - headZ;
            
            MoveAlongLine_KeepSight(camTID, headTID, headX,headY,headZ, dx,dy,dz, -1.0);
        }
        
        SetActorPosition(camTID, GetActorX(camTID), GetActorY(camTID), min(GetActorZ(camTID) + 16.0, GetActorCeilingZ(camTID)), false);
        SetActorAngle(camTID, camAngle); // this shouldn't be necessary
        
        Thing_Remove(headTID);
        Delay(1);
    }
    
    if (IsTIDUsed(camTID)) { Thing_Remove(camTID); }
}



script "BGoto_WarpItem" (void)
{
    int pln = PlayerNumber();
    if (pln == -1) { terminate; }
    
    if (CheckInventory("InWarpMenu")) { terminate; }
    
    if (!CheckActorClass(0, "SpookyGhost"))
    { 
        Print(s:"Hey, only ghosts can warp!");
        terminate;
    }
    
    if (ACS_NamedExecuteWithResult("BGoto_GetNearestPlayer", true) == -1)
    {
        Print(s:"No players to warp to.");
        terminate;
    }
    
    GiveInventory("InWarpMenu", 1);
    SetPlayerProperty(false, true, PROP_TOTALLYFROZEN);
    ACS_NamedExecuteAlways("BGoto_ChoosePlayer", 0);
}

script "BGoto_WarpMenuDone" (void) net
{
    if (!CheckInventory("InWarpMenu")) { terminate; }
    
    Delay(3);
    TakeInventory("InWarpMenu", 1);
    SetPlayerProperty(false, false, PROP_TOTALLYFROZEN);
}



#define GOTOID_CAMERA   19828
#define GOTOID_NAME     19829
#define GOTOID_CONTROLS 19827

script "BGoto_ChoosePlayer" (void) clientside
{
    int pln = PlayerNumber();
    if (pln == -1) { terminate; }
    if (pln != ConsolePlayerNumber()) { terminate; }
    
    if (!CheckActorClass(0, "SpookyGhost"))
    { 
        Print(s:"Hey, only ghosts can warp!");
        NamedRequestScriptPuke("BGoto_WarpMenuDone");
        terminate;
    }
    
    int selectedPln = ACS_NamedExecuteWithResult("BGoto_GetNearestPlayer", true);
    
    // if there's no closest player, then there's no other players
    if (selectedPln == -1)
    {
        Print(s:"No players to warp to.");
        NamedRequestScriptPuke("BGoto_WarpMenuDone");
        terminate;
    }
    
    SetHudSize(640, 480, true);
    
    str camTexName;
    int camTID      = 0;
    int dyaw        = 0;
    int camUnlocked = false;
    
    int leftDownTime  = 0;
    int rightDownTime = 0;
    
    while (true)
    {
        // input/event handling
        
        if (keyPressed(BT_FORWARD))
        {
            if (IsServer)
            {
                ACS_NamedExecuteAlways("BGoto_WarpToPlayer", 0, selectedPln, GetUserVariable(camTID, "user_angle"));
            }
            else
            {
                NamedRequestScriptPuke("BGoto_WarpToPlayer", selectedPln, GetUserVariable(camTID, "user_angle"));
            }
            break;
        }
        
        if (keyPressed(BT_BACK) || isDead(0))
        {
            break;
        }
        
        if (keyDown(BT_MOVELEFT))  { leftDownTime += 1; }
        else                       { leftDownTime  = 0; }
        
        if (keyDown(BT_MOVERIGHT)) { rightDownTime += 1; }
        else                       { rightDownTime  = 0; }
        
        // 1 if cycling right, -1 if cycling left
        int cycleDirection = 0;
        
        if (rightDownTime == 1 || (rightDownTime > 12 && (rightDownTime % 4 == 0)))
        {
            cycleDirection += 1;
        }
        
        if (leftDownTime == 1 || (leftDownTime > 12 && (leftDownTime % 4 == 0)))
        {
            cycleDirection -= 1;
        }
        
        if (!(cycleDirection || ACS_NamedExecuteWithResult("BGoto_CanWarpTo", selectedPln)))
        {
            cycleDirection = 1;
        }
        
        if (cycleDirection)
        {
            for (int i = 1; i < PLAYERMAX; i++)
            {
                int checkPln = mod(selectedPln + (i * cycleDirection), PLAYERMAX);
                if (pln == checkPln || !ACS_NamedExecuteWithResult("BGoto_CanWarpTo", checkPln)) { continue; }
                
                selectedPln = checkPln;
                Thing_Remove(camTID);
                camTID = 0;
                break;
            }
        }
        
        if (!ACS_NamedExecuteWithResult("BGoto_CanWarpTo", selectedPln))
        {
            break;
        }
        
        
        camUnlocked ^= keyPressed(BT_JUMP);
        dyaw = GetPlayerInput(-1, INPUT_YAW) * 2;
        
        
        // display
        
        int screenRatio = itof(GetScreenWidth()) / GetScreenHeight();
        int screenWidth = oldRound(480 * screenRatio);
        int centerLine  = itof(320 + (screenWidth / 2) - 150) + 0.4;
        
        if (camTID == 0 || !CheckActorClass(camTID, "GotoPlayer_Camera"))
        {
            camTID = ACS_NamedExecuteWithResult("BGoto_WarpCamera", selectedPln);
        }
        
        if (camTID)
        { 
            SetUserVariable(camTID, "user_unlocked", camUnlocked);
            SetUserVariable(camTID, "user_angle", GetUserVariable(camTID, "user_angle") + dyaw);
            
            
            camTexName = GotoCamTextures[cond(screenWidth < 1024, CAMTEX_SMALL, CAMTEX_NORMAL)][pln];
            SetCameraToTexture(camTID, camTexName, 90);
            SetFont(camTexName);
            HudMessage(s:"A"; HUDMSG_PLAIN, GOTOID_CAMERA, CR_UNTRANSLATED, centerLine, 350.2, 1.0);
            
            SetFont("SMALLFONT");
            HudMessage(s:"Now viewing: ", n:selectedPln+1; HUDMSG_PLAIN, GOTOID_NAME, CR_YELLOW, centerLine, 354.1, 1.0);
        }
        else
        {
            HudMessage(s:""; HUDMSG_PLAIN, GOTOID_CAMERA, 0,0,0,0);
            
            SetFont("SMALLFONT");
            HudMessage(s:"Not viewing: ", n:selectedPln+1; HUDMSG_PLAIN, GOTOID_NAME, CR_YELLOW, centerLine, 354.1, 1.0);
        }
        
        str aimStr = cond(camUnlocked, "Jump to \cadisable\c- camera aim", "Jump to \cdenable\c- camera aim");
        
        HudMessage(s:"Move left/right to switch players\nMove up to warp, move down to cancel\n", s:aimStr;
                HUDMSG_PLAIN, GOTOID_CONTROLS, CR_WHITE, centerLine, 366.1, 1.0);
        
        Delay(1);
    }
    
    if (camTID) { Thing_Remove(camTID); }
    
    // if we didn't get booted out, do this
    
    HudMessage(s:""; HUDMSG_PLAIN, GOTOID_CAMERA,   0,0,0,0);
    HudMessage(s:""; HUDMSG_PLAIN, GOTOID_NAME,     0,0,0,0);
    HudMessage(s:""; HUDMSG_PLAIN, GOTOID_CONTROLS, 0,0,0,0);
    
    if (IsServer)
    {
        ACS_NamedExecuteAlways("BGoto_WarpMenuDone", 0);
    }
    else
    {
        NamedRequestScriptPuke("BGoto_WarpMenuDone");
    }
}



script "BGoto_WarpToPlayer" (int warpPln, int warpAngle) net
{
    int pln = PlayerNumber();
    if (pln == -1) { terminate; }
    
    if (!CheckActorClass(0, "SpookyGhost"))
    {
        Print(s:"Hey, only ghosts can warp!");
        terminate;
    }
    
    if (!PlayerInGame(warpPln))
    {
        Print(s:"No player with ID ", d:warpPln, s:" currently ingame.");
        terminate;
    }
    
    ActivatorToPlayer(warpPln);
    int warpX     = GetActorX(0);
    int warpY     = GetActorY(0);
    int warpZ     = GetActorZ(0);
    
    ActivatorToPlayer(pln);
    Warp(0, warpX, warpY, warpZ, warpAngle, WARPF_ABSOLUTEPOSITION | WARPF_ABSOLUTEANGLE | WARPF_NOCHECKPOSITION | WARPF_STOP);
    SetActorAngle(0, warpAngle);
    SetActorPitch(0, 0);
    
    LocalAmbientSound("vile/firestrt", 127);
    FadeRange(91, 164, 220, 0.33, 91, 164, 220, 0, 0.33);
}