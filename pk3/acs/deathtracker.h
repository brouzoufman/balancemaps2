#define BDEATH_MAXDEATHS            2
#define BDEATH_MAXMARKEDSECTORS     256

int BDeath_AccumulatedDeaths[PLAYERMAX];

int BDeath_MarkedSectors[BDEATH_MAXMARKEDSECTORS][2];
int BDeath_MarkCount = 0;



function int BDeath_GetDeaths(int pln)
{
    return BDeath_AccumulatedDeaths[pln];
}

function void BDeath_SetDeaths(int pln, int i)
{
    BDeath_AccumulatedDeaths[pln] = i;
}

function void BDeath_ModDeaths(int pln, int i)
{
    BDeath_AccumulatedDeaths[pln] += i;
}


function void BDeath_MarkSector(int tag, int pln)
{
    int index = BDeath_MarkCount;
    
    if (index >= BDEATH_MAXMARKEDSECTORS)
    {
        Log(s:"\cgERROR:\ca Ran out of slots to mark sectors with (max: ", d:BDEATH_MAXMARKEDSECTORS, s:")");
        return;
    }
    
    BDeath_MarkedSectors[index][0] = tag;
    BDeath_MarkedSectors[index][1] = pln;
    BDeath_MarkCount++;
}


function void BDeath_UnmarkSector(int tag)
{
    for (int i = 0; i < BDeath_MarkCount; i++)
    {
        if (BDeath_MarkedSectors[i][0] == tag)
        {
            BDeath_MarkCount--;
            BDeath_MarkedSectors[i][0] = BDeath_MarkedSectors[BDeath_MarkCount][0];
            BDeath_MarkedSectors[i][1] = BDeath_MarkedSectors[BDeath_MarkCount][1];
        }
    }
}



int BDeath_MarkedByList[BDEATH_MAXMARKEDSECTORS];
int BDeath_MarkedByCount;

function int BDeath_CheckSectorMarks(int tag)
{
    BDeath_MarkedByCount = 0;
    
    for (int i = 0; i < BDeath_MarkCount; i++)
    {
        if (BDeath_MarkedSectors[i][0] == tag)
        {
            BDeath_MarkedByList[BDeath_MarkedByCount++] = BDeath_MarkedSectors[i][1];
        }
    }
    
    return BDeath_MarkedByCount;
}


function int BDeath_FindSectorMarks(int targetTID)
{
    BDeath_MarkedByCount = 0;
    
    int checkTID = UniqueTID();
    int checkX   = GetActorX(targetTID);
    int checkY   = GetActorY(targetTID);
    int checkZ   = GetActorZ(targetTID);
    SpawnForced("SectorMarkCheck", checkX, checkY, checkZ, checkTID);
    
    for (int i = 0; i < BDeath_MarkCount; i++)
    {
        if (ThingCountSector(0, checkTID, BDeath_MarkedSectors[i][0]) > 0)
        {
            BDeath_MarkedByList[BDeath_MarkedByCount++] = BDeath_MarkedSectors[i][1];
        }
    }
    
    return BDeath_MarkedByCount;
}

function int BDeath_MarkedByResult(int index)
{
    if (index < 0 || index >= BDeath_MarkedByCount) { return -1; }   
    return BDeath_MarkedByList[index];
}



script "BDeath_MarkSectors" (int pln, int tag1, int tag2, int tag3)
{
    if (tag1 != 0) { BDeath_MarkSector(tag1, pln); }
    if (tag2 != 0) { BDeath_MarkSector(tag2, pln); }
    if (tag3 != 0) { BDeath_MarkSector(tag3, pln); }
}

script "BDeath_UnmarkSectors" (int tag1, int tag2, int tag3)
{
    if (tag1 != 0) { BDeath_UnmarkSector(tag1); }
    if (tag2 != 0) { BDeath_UnmarkSector(tag2); }
    if (tag3 != 0) { BDeath_UnmarkSector(tag3); }
}