#define BRETURN_MAXPOINTS 128

int BReturn_PlayerPoints[PLAYERMAX];
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

// reset player Point IDs to -1
function void BReturn_ResetPlayerPoints(void)
{
    for(int i = 0; i < PLAYERMAX; i++) BReturn_PlayerPoints[i] = -1;
}

function int BReturn_GetPlayerPoint(int pln)
{
    if(pln < 0 || pln >= PLAYERMAX) return -1;
    return BReturn_PlayerPoints[pln];
}

function int BReturn_GetPlayerPointTID(int pln)
{
    if(pln < 0 || pln >= PLAYERMAX) return -1;
    return BReturn_PointData[BReturn_PlayerPoints[pln]][0];
}

function int BReturn_SetPlayerPoint(int pln, int pointID)
{
    if(pln < 0 || pln >= PLAYERMAX) return 0;
    if(pointID < 0 || pointID >= BReturn_PointCount) return 0;
    
    BReturn_PlayerPoints[pln] = pointID;
    return 1;
}



script "BReturn_TeleportPointHook" (int pointIn)
{
    Log(s:"\cqDEBUG: \cdpoint id: ", d:pointIn);
    SetResultValue(pointIn);
}

function int BReturn_TeleportToPoint(int tid, int pointID, int fog)
{
    int hookedID = ACS_NamedExecuteWithResult("BReturn_TeleportPointHook", pointID);
    
    if (hookedID < 0) { return 0; }
    if (hookedID >= BReturn_PointCount)
    {
        Log(s:"\ckWARNING:\cf Tried to teleport to invalid point ID ", d:hookedID, s:" (", d:pointID, s:" before hook)");
        return 0;
    }
    
    int pointTID = BReturn_PointData[hookedID][0];

    return TeleportFunctional(tid, pointTID, fog, false);
}



script "BReturn_Register" (int pointOrder)
{
    SetResultValue(BReturn_Register(pointOrder));
}

script "BReturn_GetPlayerPoint" (int pln)
{
    SetResultValue(BReturn_GetPlayerPoint(pln));
}

script "BReturn_GetPlayerPointTID" (int pln)
{
    SetResultValue(BReturn_GetPlayerPointTID(pln));
}

script "BReturn_SetPlayerPoint" (int pln, int pointID)
{
    SetResultValue(BReturn_SetPlayerPoint(pln, pointID));
    Print(s:"\cqDEBUG:\cd Set player ", d:pln, s:"'s point to ", d:pointID);
}

script "BReturn_TeleportToPoint" (int tid, int pointID, int fog)
{
    SetResultValue(BReturn_TeleportToPoint(tid, pointID, fog));
}