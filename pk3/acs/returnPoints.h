int BReturn_PlayerPoints[PLAYERMAX]    = {-1};
int BReturn_DefaultPoint[PLAYERMAX][6] = {{0}}; // Used when you *must* teleport but have no return point
int BReturn_PointData[BRETURN_MAXPOINTS][2];
int BReturn_PointCount = 0;

function int BReturn_Register(int pointOrder)
{
    int myTID   = defaultTID(-1);
    int myIndex = BReturn_PointCount;
    
    if (myIndex >= BRETURN_MAXPOINTS)
    {
        Log(s:"\cgERROR:\ca Ran out of slots to register return points in (max: ", d:BRETURN_MAXPOINTS, s:")");
        return 0;
    }
    
    BReturn_PointData[myIndex][0] = myTID;
    BReturn_PointData[myIndex][1] = pointOrder;
    BReturn_PointCount++;
    
    Log(s:"\cqDEBUG:\cd Registered return point ", d:myIndex, s:" (TID: ", d: myTID, s:", order: ", d:pointOrder, s:")");
    return 1;
}



function int BReturn_GetPointTID(int pointID)
{
    if(pointID < 0 || pointID >= BReturn_PointCount) return 0;
    return BReturn_PointData[pointID][0];
}

function int BReturn_GetPointOrder(int pointID)
{
    if(pointID < 0 || pointID >= BReturn_PointCount) return 0;
    return BReturn_PointData[pointID][1];
}



function int BReturn_GetPlayerPoint(int pln)
{
    if(pln < 0 || pln >= PLAYERMAX) return -1;
    return BReturn_PlayerPoints[pln];
}

function int BReturn_SetPlayerPoint(int pln, int pointID)
{
    if(pln < 0 || pln >= PLAYERMAX) return 0;
    if(pointID < 0 || pointID >= BReturn_PointCount) return 0;
    
    BReturn_PlayerPoints[pln] = pointID;
    return 1;
}

function int BReturn_SetMyPoint(int pointOrder, int direction)
{
    int pointID = BReturn_FirstPointWithOrder(pointOrder);
    if (pointID == -1) { return 0; }
    
    int pln = PlayerNumber();
    if (pln < 0) { return 0; }
    if (direction == 0) { return BReturn_SetPlayerPoint(pln, pointID); }
    
    int curPoint = BReturn_GetPlayerPoint(pln);
    int curOrder = cond(curPoint == -1, 0, BReturn_GetPointOrder(curPoint));
    
    if ((direction > 0 && pointOrder > curOrder)
     || (direction < 0 && pointOrder < curOrder))
    {
        return BReturn_SetPlayerPoint(pln, pointID);
    }
    
    return 0;
}




// Called on enter - the only time a player's return point should be negative
function int BReturn_ResetPlayerPoint(int pln)
{
    if(pln < 0 || pln >= PLAYERMAX) return 0;
    
    BReturn_PlayerPoints[pln] = -1;
    BReturn_DefaultPoint[pln][0] = GetActorX(0);
    BReturn_DefaultPoint[pln][1] = GetActorY(0);
    BReturn_DefaultPoint[pln][2] = GetActorZ(0);
    BReturn_DefaultPoint[pln][3] = GetActorAngle(0);
    BReturn_DefaultPoint[pln][4] = GetActorPitch(0);
    BReturn_DefaultPoint[pln][5] = GetActorRoll(0);
    return 1;
}



int BReturn_FoundData[BDEATH_MAXMARKEDSECTORS][3];
int BReturn_FoundCount;

function int BReturn_FindPointsWithOrder(int order)
{
    BReturn_FoundCount = 0;
    
    for (int i = 0; i < BReturn_PointCount; i++)
    {
        int thisID    = i;
        int thisTID   = BReturn_PointData[i][0];
        int thisOrder = BReturn_PointData[i][1];
        
        if (order == thisOrder)
        {
            BReturn_FoundData[BReturn_FoundCount][0] = thisID;
            BReturn_FoundData[BReturn_FoundCount][1] = thisTID;
            BReturn_FoundData[BReturn_FoundCount][2] = thisOrder;
            BReturn_FoundCount++;
        }
    }
    
    return BReturn_FoundCount;
}

function int BReturn_FirstPointWithOrder(int order)
{
    for (int i = 0; i < BReturn_PointCount; i++)
    {
        if (order == BReturn_PointData[i][1]) { return i; }
    }
    
    return -1;
}


function int BReturn_CheckResult_ID(int index)
{
    if (index < 0 || index >= BReturn_FoundCount) { return -1; }
    return BReturn_FoundData[index][0];
}

function int BReturn_CheckResult_TID(int index)
{
    if (index < 0 || index >= BReturn_FoundCount) { return 0; }
    return BReturn_FoundData[index][1];
}

function int BReturn_CheckResult_Order(int index)
{
    if (index < 0 || index >= BReturn_FoundCount) { return -1; }
    return BReturn_FoundData[index][2];
}


function int BReturn_MinOrder(void)
{
    if (BReturn_PointCount == 0) { return 0; }
    
    int ret = 0x7FFFFFFF;
    for (int i = 0; i < BReturn_PointCount; i++) { ret = min(ret, BReturn_PointData[i][1]); }
    return ret;
}

function int BReturn_MaxOrder(void)
{
    if (BReturn_PointCount == 0) { return 0; }
    
    int ret = 0x80000000;
    for (int i = 0; i < BReturn_PointCount; i++) { ret = max(ret, BReturn_PointData[i][1]); }
    return ret;
}



script "BReturn_TeleportPointHook" (int pointIn)
{
    Log(s:"\cqDEBUG: \cdpoint id: ", d:pointIn);
    SetResultValue(pointIn);
}

function int BReturn_TeleportToPoint(int tid, int pointID, int nofog)
{
    int hookedID = ACS_NamedExecuteWithResult("BReturn_TeleportPointHook", pointID);
    
    if (hookedID < 0) { return -1; }
    if (hookedID >= BReturn_PointCount)
    {
        Log(s:"\ckWARNING:\cf Tried to teleport to invalid point ", d:hookedID, s:" (", d:pointID, s:" before hook)");
        return -1;
    }
    
    int pointTID = BReturn_PointData[hookedID][0];

    if (!TeleportFunctional(tid, pointTID, !nofog, false))
    {
        Log(s:"\cgERROR:\ca Failed to teleport to point ", d:hookedID);
        return -1;
    }
    
    return hookedID;
}

function int BReturn_ReturnToPoint(int nofog)
{
    int pln = PlayerNumber();
    if (pln < 0) { return -1; }
    
    int playerPoint = BReturn_GetPlayerPoint(pln);
    
    if (playerPoint == -1)
    {
        int x   = BReturn_DefaultPoint[pln][0];
        int y   = BReturn_DefaultPoint[pln][1];
        int z   = BReturn_DefaultPoint[pln][2];
        int tid = UniqueTID();
        
        SpawnForced("MapSpot", x,y,z, tid);
        SetActorAngle(tid, BReturn_DefaultPoint[pln][3]);
        SetActorPitch(tid, BReturn_DefaultPoint[pln][4]);
        SetActorRoll(tid,  BReturn_DefaultPoint[pln][5]);
        
        int ret = TeleportFunctional(0, tid, !nofog, false);
        Thing_Remove(tid);
        
        return ret;
    }
    
    return BReturn_TeleportToPoint(0, playerPoint, nofog);
}



script "BReturn_Register" (int pointOrder) { SetResultValue(BReturn_Register(pointOrder)); }

script "BReturn_PointCount" (void) { SetResultValue(BReturn_PointCount); }

script "BReturn_GetPointTID"   (int pointID) { SetResultValue(BReturn_GetPointTID(pointID)); }
script "BReturn_GetPointOrder" (int pointID) { SetResultValue(BReturn_GetPointOrder(pointID)); }

script "BReturn_GetPlayerPoint" (int pln)                    { SetResultValue(BReturn_GetPlayerPoint(pln)); }
script "BReturn_SetPlayerPoint" (int pln, int pointID)       { SetResultValue(BReturn_SetPlayerPoint(pln, pointID)); }
script "BReturn_SetMyPoint"     (int pointOrder, int onlyUp) { SetResultValue(BReturn_SetMyPoint(pointOrder, onlyUp)); }

script "BReturn_FindPointsWithOrder" (int order) { SetResultValue(BReturn_FindPointsWithOrder(order)); }
script "BReturn_FirstPointWithOrder" (int order) { SetResultValue(BReturn_FirstPointWithOrder(order)); }

script "BReturn_CheckResult_ID"    (int index) { SetResultValue(BReturn_CheckResult_ID(index)); }
script "BReturn_CheckResult_TID"   (int index) { SetResultValue(BReturn_CheckResult_TID(index)); }
script "BReturn_CheckResult_Order" (int index) { SetResultValue(BReturn_CheckResult_Order(index)); }

script "BReturn_MinOrder" (void) { SetResultValue(BReturn_MinOrder()); }
script "BReturn_MaxOrder" (void) { SetResultValue(BReturn_MaxOrder()); }

script "BReturn_TeleportToPoint" (int tid, int pointID, int nofog) { SetResultValue(BReturn_TeleportToPoint(tid, pointID, nofog)); }
script "BReturn_ReturnToPoint"   (int nofog)                       { SetResultValue(BReturn_ReturnToPoint(nofog)); }