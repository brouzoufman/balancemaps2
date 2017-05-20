#define BDEATH_MAXDEATHS            2
#define BDEATH_MAXMARKEDSECTORS     256

int BDeath_AccumulatedDeaths[PLAYERMAX];

int BDeath_MarkedSectors[BDEATH_MAXMARKEDSECTORS][3];
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


function void BDeath_MarkSector(int id, int tag, int pln)
{
    int index = BDeath_MarkCount;
    
    if (index >= BDEATH_MAXMARKEDSECTORS)
    {
        Log(s:"\cgERROR:\ca Ran out of slots to mark sectors with (max: ", d:BDEATH_MAXMARKEDSECTORS, s:")");
        return;
    }
    
    BDeath_MarkedSectors[index][0] = tag;
    BDeath_MarkedSectors[index][1] = pln;
    BDeath_MarkedSectors[index][2] = id;
    BDeath_MarkCount++;
}


function void BDeath_UnmarkSector(int id, int tag)
{
    for (int i = 0; i < BDeath_MarkCount; i++)
    {
        int thisTag = BDeath_MarkedSectors[i][0];
        int thisID  = BDeath_MarkedSectors[i][2];
        
        if (thisTag == tag && thisID == id)
        {
            BDeath_MarkCount--;
            BDeath_MarkedSectors[i][0] = BDeath_MarkedSectors[BDeath_MarkCount][0];
            BDeath_MarkedSectors[i][1] = BDeath_MarkedSectors[BDeath_MarkCount][1];
            BDeath_MarkedSectors[i][2] = BDeath_MarkedSectors[BDeath_MarkCount][2];
            i--; // Check this index again, since it's got something new in it
        }
    }
}


function void BDeath_UnmarkByID(int id)
{
    for (int i = 0; i < BDeath_MarkCount; i++)
    {
        int thisID  = BDeath_MarkedSectors[i][2];
        
        if (thisID == id)
        {
            BDeath_MarkCount--;
            BDeath_MarkedSectors[i][0] = BDeath_MarkedSectors[BDeath_MarkCount][0];
            BDeath_MarkedSectors[i][1] = BDeath_MarkedSectors[BDeath_MarkCount][1];
            BDeath_MarkedSectors[i][2] = BDeath_MarkedSectors[BDeath_MarkCount][2];
            i--; // Check this index again, since it's got something new in it
        }
    }
}

function int BDeath_Disassociate(int pln)
{
    int ret = 0;
    
    for (int i = 0; i < BDeath_MarkCount; i++)
    {
        if (BDeath_MarkedSectors[i][1] == pln)
        {
            BDeath_MarkedSectors[i][1] = -1;
            ret++;
        }
    }
    
    return ret;
}



int BDeath_MarkedByData[BDEATH_MAXMARKEDSECTORS][3];
int BDeath_MarkedByCount;

function int BDeath_CheckSectorMarks(int tag)
{
    BDeath_MarkedByCount = 0;
    
    for (int i = 0; i < BDeath_MarkCount; i++)
    {
        int thisTag = BDeath_MarkedSectors[i][0];
        int thisPln = BDeath_MarkedSectors[i][1];
        int thisID  = BDeath_MarkedSectors[i][2];
        
        if (tag == thisTag)
        {
            BDeath_MarkedByData[BDeath_MarkedByCount][0] = thisTag;
            BDeath_MarkedByData[BDeath_MarkedByCount][1] = thisPln;
            BDeath_MarkedByData[BDeath_MarkedByCount][2] = thisID;
            BDeath_MarkedByCount++;
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
        int thisTag = BDeath_MarkedSectors[i][0];
        int thisPln = BDeath_MarkedSectors[i][1];
        int thisID  = BDeath_MarkedSectors[i][2];
        
        if (ThingCountSector(0, checkTID, thisTag) > 0)
        {
            BDeath_MarkedByData[BDeath_MarkedByCount][0] = thisTag;
            BDeath_MarkedByData[BDeath_MarkedByCount][1] = thisPln;
            BDeath_MarkedByData[BDeath_MarkedByCount][2] = thisID;
            BDeath_MarkedByCount++;
        }
    }
    
    return BDeath_MarkedByCount;
}

function int BDeath_CheckResult_Tag(int index)
{
    if (index < 0 || index >= BDeath_MarkedByCount) { return -1; }
    return BDeath_MarkedByData[index][0];
}

function int BDeath_CheckResult_Player(int index)
{
    if (index < 0 || index >= BDeath_MarkedByCount) { return -1; }
    return BDeath_MarkedByData[index][1];
}

function int BDeath_CheckResult_ID(int index)
{
    if (index < 0 || index >= BDeath_MarkedByCount) { return -1; }
    return BDeath_MarkedByData[index][2];
}



script "BDeath_MarkSectors" (int id, int pln, int tag1, int tag2)
{
    if (tag1 != 0) { BDeath_MarkSector(id, tag1, pln); }
    if (tag2 != 0) { BDeath_MarkSector(id, tag2, pln); }
}

script "BDeath_UnmarkSectors" (int id, int tag1, int tag2)
{
    if (tag1 != 0) { BDeath_UnmarkSector(id, tag1); }
    if (tag2 != 0) { BDeath_UnmarkSector(id, tag2); }
}

script "BDeath_UnmarkByID" (int id)
{
    BDeath_UnmarkByID(id);
}