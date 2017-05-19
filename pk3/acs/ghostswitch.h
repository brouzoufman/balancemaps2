#define BSWITCH_MAXSWITCHES     128
#define BSWITCH_CHECKDIST       320.0

int BSwitch_RegisteredSwitches[BSWITCH_MAXSWITCHES][3];
int BSwitch_SwitchCount = 0;


script "BSwitch_Register" (int scriptNum, int cooldownTics)
{
    int myTID   = defaultTID(-1);
    int myIndex = BSwitch_SwitchCount;
    
    if (myIndex >= BSWITCH_MAXSWITCHES)
    {
        Log(s:"\cgERROR:\ca Ran out of slots to register ghost switches in (max: ", d:BSWITCH_MAXSWITCHES, s:")");
        terminate;
    }
    
    BSwitch_RegisteredSwitches[myIndex][0] = myTID;
    BSwitch_RegisteredSwitches[myIndex][1] = scriptNum;
    BSwitch_RegisteredSwitches[myIndex][2] = cooldownTics;
    BSwitch_SwitchCount++;
    
    Log(s:"\cqDEBUG:\cd Registered ghost switch ", d:myIndex, s:" (TID: ", d: myTID, s:", script: ", d:scriptNum, s:", cooldown: ", d:cooldownTics, s:")");
}


function int BSwitch_FindNearest(int myX, int myY, int myZ, int ignoreCooldown)
{
    // Anything not in the box defined by these can be quickly thrown away
    int xmin = safeAdd(myX, -BSWITCH_CHECKDIST), xmax = safeAdd(myX, BSWITCH_CHECKDIST);
    int ymin = safeAdd(myY, -BSWITCH_CHECKDIST), ymax = safeAdd(myY, BSWITCH_CHECKDIST);
    int zmin = safeAdd(myZ, -BSWITCH_CHECKDIST), zmax = safeAdd(myZ, BSWITCH_CHECKDIST);
    
    int curNearestTID  = -1;
    int curNearestDist = 0x7FFFFFFF;
    
    for (int i = 0; i < BSwitch_SwitchCount; i++)
    {
        int thisTID = BSwitch_RegisteredSwitches[i][0];
        
        if (!ignoreCooldown && CheckActorInventory(thisTID, "GhostSwitchOnCooldown"))
        {
            continue;
        }
        
        int thisX   = GetActorX(thisTID);
        int thisY   = GetActorY(thisTID);
        int thisZ   = GetActorZ(thisTID);
        
        if (thisX >= xmin && thisY >= ymin && thisZ >= zmin
         && thisX <= xmax && thisY <= ymax && thisZ <= zmax)
        {
            int thisDist = distance(myX, myY, myZ, thisX, thisY, thisZ);
            if (thisDist > BSWITCH_CHECKDIST || thisDist > curNearestDist) { continue; }
            
            curNearestTID  = thisTID;
            curNearestDist = thisDist;
        }
    }
    
    return curNearestTID;
}


script "BSwitch_ActivateNearest" (void)
{
    int pln = PlayerNumber();
    if (pln == -1)
    {
        Log(s:"\ckWARNING:\cf Non-player ", n:0, s:" (TID ", d:ActivatorTID(), s:") tried to activate ghost switches");
        terminate;
    }

    int myX = GetActorX(0);
    int myY = GetActorY(0);
    int myZ = GetActorZ(0);
    
    int nearestSwitch = BSwitch_FindNearest(myX, myY, myZ, false);
}