//ingame timer, and storing of server records

#define RECORDCOUNT 5

//will replace these with custom colours later
int BT_RecordColors[RECORDCOUNT] = {CR_GOLD, CR_WHITE, CR_TAN, CR_DARKGRAY, CR_BLACK};

int BT_RecordTimes[RECORDCOUNT]; //seconds
int BT_RecordNames[RECORDCOUNT];
int BT_RecordDates[RECORDCOUNT];
int BT_LastRecordUpdate = -1;

int BT_FinishTimes[PLAYERMAX];


function str BTimer_TimeString(int tics, int withMS)
{
    int negativeTime;
    if (tics < 0) { negativeTime = true; tics = -tics; }
    
    int seconds = (tics / 35)   % 60;
    int minutes = (tics / 2100) % 60;
    int hours   = (tics / 126000);
    
    str hourStr = "", minuteStr = "", secondStr = "";
    
    if (hours > 0)
    {
        hourStr   = StrParam(d:hours, s:":");
        minuteStr = StrParam(s:cond(minutes < 10, "0", ""), d:minutes, s:":");
    }
    else
    {
        minuteStr = StrParam(d:minutes, s:":");
    }
    
    if (withMS)
    {
        int fracSeconds = itof(tics % 2100) / 35;
        secondStr = StrParam(s:cond(seconds < 10, "0", ""), f:fracSeconds);
        
        // no fractional part, add period
        if ((fracSeconds & 0xFFFF) == 0)
        {
            secondStr = StrParam(s:secondStr, s:".");
        }
        
        // pad past decimal point for consistency
        int decimalPos = strstr(secondStr, ".");
        secondStr = padStringR(secondStr, "0", decimalPos+6);
    }
    else
    {
        secondStr = StrParam(s:cond(seconds < 10, "0", ""), d:seconds);
    }
    
    return StrParam(s:hourStr, s:minuteStr, s:secondStr);
}


function void BTimer_LoadRecords(void)
{
    str table_times = StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_times");
    str table_dates = StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_dates");
    str table_names = StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_names");
    
    int sortedTimes     = SortDBEntries(table_times, RECORDCOUNT, 0, 0);
    int sortedTimeCount = CountDBResults(sortedTimes);
    
    for (int i = 0; i < min(RECORDCOUNT, sortedTimeCount); i++)
    {
        str nameKey = GetDBResultKeyString(sortedTimes, i);
        
        int recordTime = GetDBEntry(table_times, nameKey);
        str recordDate = GetDBEntryString(table_dates, nameKey);
        str recordName = GetDBEntryString(table_names, nameKey);
        
        BT_RecordTimes[i] = recordTime;
        BT_RecordNames[i] = recordName;
        BT_RecordDates[i] = recordDate;
    }
    
    FreeDBResults(sortedTimes);
    
    // clear out empty records just in case
    for (int j = i; j < RECORDCOUNT; j++)
    {
        BT_RecordTimes[j] = -1;
        BT_RecordNames[j] = "";
        BT_RecordDates[j] = "";
    }
    
    BT_LastRecordUpdate = Timer();
}


function int BTimer_GetRecord(str name)
{
    str table_times = StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_times");
    
    str cleanName = strLower(cleanString(name));
    int curRecord = GetDBEntry(table_times, cleanName);
    
    if (curRecord > 0) { return curRecord; }
    return -1;
}


function int BTimer_UpdateRecord(str name, int time)
{
    // either you're cheating or a bug occured, either way *no*
    if (time <= 0) { return false; }
    
    str table_times = StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_times");
    str table_dates = StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_dates");
    str table_names = StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_names");
    
    str cleanName = strLower(cleanString(name));
    int curRecord = GetDBEntry(table_times, cleanName);

    if ((curRecord <= 0) || (curRecord > time))
    {
        BeginDBTransaction();
        SetDBEntry(table_times, cleanName, time);
        SetDBEntryString(table_dates, cleanName, Strftime(SystemTime(), "%Y-%m-%d %H:%M:%S %z"));
        SetDBEntryString(table_names, cleanName, name);
        EndDBTransaction();
        
        return true;
    }
    
    return false;
}



script "BTimer_Open" OPEN
{
    int lastUpdate = -1;
    
    BTimer_LoadRecords();
    
    SetHudSize(800, 600, true);
    
    while (true)
    {
        int t = Timer();
        
        if (t == 1 || (t / 35) > ((t - 1) / 35))
        {
            SetFont("BIGFONT");
            HudMessageBold(s:BTimer_TimeString(t, false);
                HUDMSG_PLAIN, 198, CR_DARKGREEN, 20.1, 318.0, 0);
        }
        
        if (lastUpdate < BT_LastRecordUpdate)
        {
            SetFont("SMALLFONT");
            HudMessage(s:"Server records";
                HUDMSG_PLAIN, 199, CR_DARKGREEN, 20.1, 360.0, 0);
            
            for (int i = 0; i < RECORDCOUNT; i++)
            {
                int recordTime = BT_RecordTimes[i];
                str recordName = BT_RecordNames[i];
                
                if (recordTime > 0)
                {
                    HudMessage(s:"#", d:i+1, s:": ", s:recordName;
                        HUDMSG_PLAIN, 201 + (i*2), BT_RecordColors[i], 20.1, 373.0 + (i * 23.0), 0);
                    
                    HudMessage(s:"Time: \c[gray]", s:BTimer_TimeString(recordTime, true);
                        HUDMSG_PLAIN, 202 + (i*2), BT_RecordColors[i], 20.1, 382.0 + (i * 23.0), 0);
                }
                else
                {
                    HudMessage(s:""; HUDMSG_PLAIN, 201 + (i*2), 0,0,0,0);
                    HudMessage(s:""; HUDMSG_PLAIN, 202 + (i*2), 0,0,0,0);
                }
            }
            
            lastUpdate = BT_LastRecordUpdate;
        }
        
        Delay(1);
    }
}



script "BTimer_Display" ENTER
{
    int pln = PlayerNumber();
    SetHudSize(800, 600, true);
    SetFont("BIGFONT");
    
    //display timer stuff
    while (true)
    {
        int t = Timer();
        
        if (t == 1 || (t / 35) > ((t - 1) / 35))
        {
            int finishTime = BT_FinishTimes[pln];
            int showTime   = cond(finishTime > 0, finishTime, t);
            
            HudMessage(s:BTimer_TimeString(showTime, false);
                HUDMSG_FADEOUT, 200, CR_GREEN, 20.1, 335.0, 1.25, 0.5);
        }
        
        Delay(1);
    }
}



script "BTimer_Finish" (void)
{
    if (CheckInventory("ShouldBeGhost")) { terminate; }
    
    int pln = PlayerNumber();
    if (pln < 0 || BT_FinishTimes[pln] > 0) { terminate; }
    
    str playerName = StrParam(n:0);
    int playerTime = Timer();
    
    BT_FinishTimes[pln] = playerTime;
    
    if (BTimer_UpdateRecord(playerName, playerTime))
    {
        BTimer_LoadRecords();
    }
}



//----------------
//map exit scripts
//----------------

function int BTimer_PlayersLeft(void)
{
    int ret = 0;
    
    for (int i = 0; i < PLAYERMAX; i++)
    {
        if (!PlayerInGame(i)|| PlayerIsBot(i)
         || BT_FinishTimes[i] > 0
         || CheckActorInventory(BMaps_PlayerTIDs[i], "ShouldBeGhost"))
        {
            continue;
        }
        
        ret++;
    }
    
    return ret;
}



script "BMaps_Exit" (int countdown)
{
    int pln = PlayerNumber();
    
    if (CheckInventory("ShouldBeGhost") || pln < 0)
    {
        SetResultValue(0);
        terminate;
    }
    
    //if the mapper hasn't put a trigger in, make sure a timer is registered
    ACS_NamedExecuteWithResult("BTimer_Finish");
    
    if (BTimer_PlayersLeft() > 0)
    {
        ACS_NamedExecute("BMaps_Exit_Countdown", 0, countdown);
    }
    else
    {
        Exit_Normal(0);
    }
}



script "BMaps_Exit_Countdown" (int countdown)
{
    SetHudSize(800, 600, true);
    SetFont("BIGFONT");
    
    for (int i = 0; i < countdown; i++)
    {
        if (BTimer_PlayersLeft() <= 0) { break; }
        
        HudMessageBold(s:"Map ends in ", d:countdown, s:"...";
            HUDMSG_PLAIN, 197, CR_RED, 400.0, 100.0, 0);
        
        Delay(35);
    }
    
    Exit_Normal(0);
}