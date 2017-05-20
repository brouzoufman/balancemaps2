#define BRETURN_MAXPOINTS 128

int BReturn_PlayerPoints[PLAYERMAX];
int BReturn_PointTIDs[BRETURN_MAXPOINTS];

function int BReturn_RegisterPoint(int pointID, int pointTID)
{
	if(pointID < 0 || pointID >= BRETURN_MAXPOINTS) return 0;
	
	BReturn_PointTIDs[pointID] = pointTID;
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
	return BReturn_PointTIDs[BReturn_PlayerPoints[pln]];
}

function int BReturn_SetPlayerPoint(int pln, int pointID)
{
	if(pln < 0 || pln >= PLAYERMAX) return 0;
	if(pointID < 0 || pointID >= BRETURN_MAXPOINTS) return 0;
	
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
	if(pointID < 0 || pointID >= BRETURN_MAXPOINTS) return 0;
    
    int hookedID = ACS_NamedExecuteWithResult("BReturn_TeleportPointHook", pointID);
	int pointTID = BReturn_PointTIDs[hookedID];

	return TeleportFunctional(tid, pointTID, fog, 0);
}



script "BReturn_RegisterPoint" (int pointID, int pointTID)
{
	SetResultValue(BReturn_RegisterPoint(pointID, pointTID));
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