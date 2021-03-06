#define BONFIRE_INVULNTICS      35
#define BONFIRE_COOLDOWNTICS    70

script "Bonfire_Fire" (void)
{
    int x = GetActorX(0);
    int y = GetActorY(0);
    int z = GetActorZ(0);
    
    int tid   = UniqueTID();
    int myTID = defaultTID(0);
    
    for (int i = 0; i < 5; i++)
    {
        SpawnForced("BonfireFlame", x,y,z+6.0, tid, i*(256/5));
        SetActivator(tid);
        
        int scale = random(0.75, 1.4);
        SetActorProperty(0, APROP_Health, random(1, 6));
        SetActorProperty(0, APROP_ScaleX, scale);
        SetActorProperty(0, APROP_ScaleY, scale);
        SetPointer(AAPTR_TARGET, myTID);
        Thing_ChangeTID(0, 0);
    }
}


script "Bonfire_RadiusHeal" (int radius, int pointOrder, int whenToSet)
{
    int myX     = GetActorX(0);
    int myY     = GetActorY(0);
    int myZ     = GetActorZ(0) + 24.0;
    int myTID   = defaultTID(0);
    
    radius = itof(radius);
    
    int setDirection = 0;
    
    switch (whenToSet)
    {
        case 1: setDirection =  1; break;
        case 2: setDirection = -1; break;
    }
    
    for (int i = 0; i < PLAYERMAX; i++)
    {
        if (!PlayerInGame(i)) { continue; }
        
        SetActivatorToPlayer(i);
        if (isDead(0) || CheckInventory("ShouldBeGhost")) { continue; }
        
        // check from the bonfire, since the edge of the player might be sticking out
        if (!CheckSight(myTID, 0, 0)) { continue; }
        
        int hisX = GetActorX(0);
        int hisY = GetActorY(0);
        int hisZ = GetActorZ(0);
        int hisWidth  = GetActorProperty(0, APROP_Radius);
        int hisHeight = GetActorProperty(0, APROP_Height);
        
        int hisX_min = safeAdd(hisX, -hisWidth), hisX_max = safeAdd(hisX, hisWidth);
        int hisY_min = safeAdd(hisY, -hisWidth), hisY_max = safeAdd(hisY, hisWidth);
        int hisZ_min = hisZ,                     hisZ_max = safeAdd(hisZ, hisHeight);
        
        int hisX_closest = middle(hisX_min, myX, hisX_max);
        int hisY_closest = middle(hisY_min, myY, hisY_max);
        int hisZ_closest = middle(hisZ_min, myZ, hisZ_max);
        
        int dist = distance(myX, myY, myZ, hisX_closest, hisY_closest, hisZ_closest);
        
        if (dist <= radius)
        {
            ACS_NamedExecuteWithResult("Bonfire_Heal");
            if (whenToSet > 0) { BReturn_SetMyPoint(pointOrder, setDirection); }
        }
    }
}


script "Bonfire_Heal" (void)
{
    giveHealth(getMaxHealth());
    if (CheckInventory("BonfireCooldownTicker")) { terminate; }
    
    if (CheckInventory("BonfireInvulnTicker"))
    {
        SetInventory("BonfireInvulnTicker", BONFIRE_INVULNTICS);
        terminate;
    }
    
    ACS_NamedExecuteWithResult("Bonfire_HealEffect");
    GiveInventory("BonfireInvulnerability", 1);
    SetInventory("BonfireInvulnTicker", BONFIRE_INVULNTICS);
    
    while (CheckInventory("BonfireInvulnTicker"))
    {
        Delay(1);
        TakeInventory("BonfireInvulnTicker", 1);
    }
    
    TakeInventory("BonfireInvulnerability", 0x7FFFFFFF);
    SetInventory("BonfireCooldownTicker", BONFIRE_COOLDOWNTICS);
    
    while (CheckInventory("BonfireCooldownTicker"))
    {
        Delay(1);
        TakeInventory("BonfireCooldownTicker", 1);
    }
}

script "Bonfire_HealEffect" (void) clientside
{
    LocalAmbientSound("balancemaps/bonfire_heal_local", 96);
    ActivatorSound("balancemaps/bonfire_heal_world", 96);
    Delay(4);
    
    int myX = GetActorX(0);
    int myY = GetActorY(0);
    int myZ = GetActorZ(0) + 16.0;
    int tid = UniqueTID();
    
    int myVelX = GetActorVelX(0) / 3;
    int myVelY = GetActorVelY(0) / 3;
    int myVelZ = GetActorVelZ(0) / 3;
    
    int i, ang;
    
    for (i = 0; i < 6; i++)
    {
        ang = itof(i) / 6 + (1.0 / 12);
        
        SpawnForced("BonfireHealFlame_Inner", myX, myY, myZ, tid);
        SetActorVelocity(tid, myVelX + 2 * cos(ang), myVelY + 2 * sin(ang), 0, false, false);
        SetActorProperty(0, APROP_Alpha, GetActorProperty(0, APROP_Alpha) + random(0, 0.2));
        Thing_ChangeTID(tid, 0);
    }
    
    for (i = 0; i < 15; i++)
    {
        ang = itof(i) / 15 + (1.0 / 30);
        
        SpawnForced("BonfireHealFlame_Outer", myX, myY, myZ, tid);
        SetActorVelocity(tid, myVelX + 6 * cos(ang), myVelY + 6 * sin(ang), 1.0, false, false);
        SetActorProperty(0, APROP_Alpha, GetActorProperty(0, APROP_Alpha) + random(0, 0.2));
        Thing_ChangeTID(tid, 0);
    }
    
    FadeTo(255, 159, 64,  0.2,  3.0/35);
    Delay(3);
    FadeTo(255, 212, 128, 0.6,  6.0/35);
    Delay(5);
    FadeTo(255, 225, 166, 0.7,  6.0/35);
    Delay(5);
    FadeTo(255, 234, 191, 0.75, 6.0/35);
    Delay(16);
    FadeTo(255, 159, 64,  0,   52.5/35);
}


script "Bonfire_FlameSize" (void)
{
    while (true)
    {
        int myScale = GetActorProperty(0, APROP_ScaleY);
        int scaleDirection = random(-1, 1);
        
        if (myScale < 0.75) { scaleDirection = max(0, scaleDirection); }
        if (myScale > 1.5)  { scaleDirection = min(0, scaleDirection); }
        
        int scalar = 0.05 * scaleDirection;
        
        if (scalar)
        {
            for (int i = 0; i < 5; i++)
            {
                myScale += scalar;
                
                SetActorProperty(0, APROP_ScaleX, myScale);
                SetActorProperty(0, APROP_ScaleY, myScale);
                Delay(8);
            }
        }
        
        Delay(random(1,8) * 8);
    }
}


script "BMaps_TurnFlamethrower" (int yawspeed, int pitchspeed)
{
    yawspeed   = itofDiv(yawspeed,   35 * 360);
    pitchspeed = itofDiv(pitchspeed, 35 * 360);
    
    SetUserVariable(0, "user_angle", GetActorAngle(0));
    SetUserVariable(0, "user_pitch", GetActorPitch(0));
    
    while (true)
    {
        int curAngle = GetActorAngle(0);
        int curPitch = GetActorPitch(0);
        
        int desiredAngle = GetUserVariable(0, "user_angle");
        int desiredPitch = GetUserVariable(0, "user_pitch");
        
        int angDiff   = angleDifference(curAngle, desiredAngle);
        int pitchDiff = angleDifference(curPitch, desiredPitch);
        
        if (angDiff != 0)
        {
            if (yawspeed == 0 || abs(angDiff) <= yawspeed)
            {
                SetActorAngle(0, desiredAngle);
            }
            else if (angDiff < 0)
            {
                SetActorAngle(0, curAngle - yawspeed);
            }
            else
            {
                SetActorAngle(0, curAngle + yawspeed);
            }
        }
        
        if (pitchDiff != 0)
        {
            if (pitchspeed == 0 || abs(pitchDiff) <= pitchspeed)
            {
                SetActorPitch(0, desiredPitch);
            }
            else if (pitchDiff < 0)
            {
                SetActorPitch(0, curPitch - pitchspeed);
            }
            else
            {
                SetActorPitch(0, curPitch + pitchspeed);
            }
        }
        
        Delay(1);
    }
}

script "BMaps_RocketJumpOn" (void)
{
    SetActivatorToTarget(0);
    int pln = PlayerNumber();
    
    for (int i = 0; i < PLAYERMAX; i++)
    {
        if (!PlayerInGame(i)) { continue; }
        SetActivatorToPlayer(i);
        
        if (pln == i)
        {
            GiveInventory("RJInvulnerability", 1);
        }
        else if (CheckFlag(0, "SHOOTABLE"))
        {
            GiveInventory("RJMakeUnshootable", 1);
        }
    }
}

script "BMaps_RocketJumpOff" (void)
{
    for (int i = 0; i < PLAYERMAX; i++)
    {
        if (!PlayerInGame(i)) { continue; }
        SetActivatorToPlayer(i);
        
        if (CheckInventory("RJInvulnerability"))
        {
            TakeInventory("RJInvulnerability", 1);
        }
        
        if (CheckInventory("RJUnshootable"))
        {
            GiveInventory("RJMakeShootable", 1);
        }
    }
}