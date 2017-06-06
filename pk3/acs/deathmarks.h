int BMark_PlayerMarks[PLAYERMAX][BMARK_MAXDEATHMARKS][2];
int BMark_PlayerMarkCount[PLAYERMAX] = {0};

function int BMark_MarkPlayer(int markedPln, int markingPln, int time)
{
    int myIndex = BMark_PlayerMarkCount[markedPln];
    
    // Don't log an error here; there is a possibility, no matter how slight,
    //  that the guy did in fact just take 130 or so fireballs to the face
    if (myIndex >= BMARK_MAXDEATHMARKS) { return -1; }
    
    BMark_PlayerMarks[markedPln][myIndex][0] = markingPln;
    BMark_PlayerMarks[markedPln][myIndex][1] = Timer() + time;
    
    BMark_PlayerMarkCount[markedPln]++;
    return myIndex;
}


function void BMark_RemoveMark(int pln, int index)
{
    int top = --BMark_PlayerMarkCount[pln];
    BMark_PlayerMarks[pln][index][0] = BMark_PlayerMarks[pln][top][0];
    BMark_PlayerMarks[pln][index][1] = BMark_PlayerMarks[pln][top][1];
}


function void BMark_PruneMarks(int pln)
{
    int curTime = Timer();
    
    for (int i = 0; i < BMark_PlayerMarkCount[pln]; i++)
    {
        int markingPln = BMark_PlayerMarks[pln][i][0];
        int indexTime  = BMark_PlayerMarks[pln][i][1];
        
        if (!PlayerInGame(markingPln) || curTime > indexTime)
        {
            BMark_RemoveMark(pln, i--);
        }
    }
}


function void BMark_ClearMarks(int pln)
{
    BMark_PlayerMarkCount[pln] = 0;
}


script "BMark_GetPlayerMark" (void)
{
    int ret = CheckInventory("MarkedByPlayer") - 1;
    if (ret > -1) { SetResultValue(ret); terminate; }
    
    if (!SetActivator(0, AAPTR_TARGET)) { SetResultValue(-1); terminate; }
    
    ret = PlayerNumber();
    if (ret < 0) { ret = CheckInventory("MarkedByPlayer") - 1; }
    
    SetResultValue(ret);
}


script "BMark_MarkFromProjectile" (int time, int ptr)
{
    int playerMark = ACS_NamedExecuteWithResult("BMark_GetPlayerMark");
    if (playerMark < 0) { terminate; }
    
    if (!SetActivator(0, ptr)) { terminate; }
    
    int myPln = PlayerNumber();
    if (myPln < 0) { terminate; }
    
    BMark_MarkPlayer(myPln, playerMark, time);
}


script "BMark_MarkInRadius" (int time, int radius, int sphere)
{
    radius = itof(radius);
    
    int myX = GetActorX(0);
    int myY = GetActorY(0);
    int myZ = GetActorZ(0);
    
    int playerMark = ACS_NamedExecuteWithResult("BMark_GetPlayerMark");
    if (playerMark < 0) { terminate; }
    
    int xmin = safeAdd(myX, -radius), xmax = safeAdd(myX, radius);
    int ymin = safeAdd(myY, -radius), ymax = safeAdd(myY, radius);
    int zmin = safeAdd(myZ, -radius), zmax = safeAdd(myZ, radius);
    
    for (int pln = 0; pln < PLAYERMAX; pln  ++)
    {
        if (!PlayerInGame(pln)) { continue; }
        
        int plTID = BMaps_PlayerTIDs[pln];
        int plX   = GetActorX(plTID);
        int plY   = GetActorY(plTID);
        int plZ   = GetActorZ(plTID);
        
        int plRadius = GetActorProperty(plTID, APROP_Radius);
        int plHeight = GetActorProperty(plTID, APROP_Height);
        
        int plXMin = safeAdd(plX, -plRadius), plXMax = safeAdd(plX, plRadius);
        int plYMin = safeAdd(plY, -plRadius), plYMax = safeAdd(plY, plRadius);
        int plZMin = plZ,                     plZMax = safeAdd(plZ, plHeight);
        
        int collides = false;
        
        if (sphere)
        {
            // Find closest point in player AABB to sphere center
            int xNearest = middle(plXMin, myX, plXMax);
            int yNearest = middle(plYMin, myY, plYMax);
            int zNearest = middle(plZMin, myZ, plZMax);
            
            // Eliminate if that point isn't even in bounding box
            if (xNearest < xmin || yNearest < ymin || zNearest < zmin
             || xNearest > xmax || yNearest > ymax || zNearest > zmax)
            {
                collides = false;
            }
            else
            {
                collides = distance(myX, myY, myZ, xNearest, yNearest, zNearest) <= radius;
            }
        }
        else
        {
            int collidesOnX = xmin <= plXMax && xmax >= plXMin;
            int collidesOnY = ymin <= plYMax && ymax >= plYMin;
            int collidesOnZ = xmin <= plXMax && xmax >= plXMin;
            
            collides = collidesOnX && collidesOnY && collidesOnZ;
        }
        
        if (collides)
        {
            BMark_MarkPlayer(pln, playerMark, time);
        }
    }
}

script "BMark_DealDamage" (int damage, int ptr, int nothrust)
{
    int playerMark = ACS_NamedExecuteWithResult("BMark_GetPlayerMark");
    str damageType = GetActorProperty(0, APROP_DamageType);
    int myTID      = defaultTID(-1);
    
    if (!SetActivator(0, ptr)) { terminate; }
    int hisTID  = defaultTID(-1);
    int hisMass = GetActorProperty(0, APROP_Mass);
    int hisPln  = PlayerNumber();
    SetActivator(myTID);
    
    if (playerMark > -1 && hisPln > -1)
    {
        int markID = BMark_MarkPlayer(hisPln, playerMark, 0);
        Thing_Damage2(hisTID, damage, damageType);
        BMark_RemoveMark(hisPln, markID);
    }
    else
    {
        Thing_Damage2(hisTID, damage, damageType);
    }
    
    if (!(nothrust || GetActorProperty(hisTID, APROP_Invulnerable)))
    {
        int thrustAngle = VectorAngle(GetActorX(hisTID) - GetActorX(myTID),
                                      GetActorY(hisTID) - GetActorY(myTID));
        
        int thrustPower = (itof(damage) * 10) / hisMass;
        
        SetActorVelocity(hisTID, FixedMul(thrustPower, cos(thrustAngle)),
                                 FixedMul(thrustPower, sin(thrustAngle)),
                                 0, true, true);
    }
}