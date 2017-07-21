//ingame timer, and storing of server records

#define RECORDCOUNT     5
#define DB_COLUMNCOUNT  4

#define RDATA_NAME      0
#define RDATA_TIME      1
#define RDATA_DATE      2
#define RDATA_ACCOUNT   3    // I don't actually use this in the DB

int BT_RecordColors[RECORDCOUNT]  = {"NewGold", "Silver", "Bronze", "Iron", "Rust"};
int BT_DBSuffixes[DB_COLUMNCOUNT] = {"names", "times", "dates", "accounts"};

int BT_RecordData[RECORDCOUNT][DB_COLUMNCOUNT];



function str BTimer_DBColumnName(int col)
{
    if (col < 0 || col >= DB_COLUMNCOUNT) { return ""; }
    str ret = StrParam(s:"bmaps", d:VERSION_MAJOR, s:".", d:VERSION_MINOR,
                        s:"_", n:PRINTNAME_LEVEL, s:"_", s:BT_DBSuffixes[col]);
    
    // for some reason this is necessary
    return strLower(ret);
}


function void BTimer_LoadRecords(void)
{
    str table_times = BTimer_DBColumnName(RDATA_TIME);
    str table_dates = BTimer_DBColumnName(RDATA_DATE);
    str table_names = BTimer_DBColumnName(RDATA_NAME);
    
    int sortedTimes     = SortDBEntries(table_times, RECORDCOUNT, 0, 0);
    int sortedTimeCount = CountDBResults(sortedTimes);
    
    for (int i = 0; i < min(RECORDCOUNT, sortedTimeCount); i++)
    {
        str nameKey = GetDBResultKeyString(sortedTimes, i);
        
        int recordTime    = GetDBEntry(table_times, nameKey);
        str recordDate    = GetDBEntryString(table_dates, nameKey);
        str recordName    = GetDBEntryString(table_names, nameKey);
        str recordAccount = "";
        
        if (GetChar(nameKey, 0) == 30)
        {
            recordAccount = StrMid(nameKey, 1, StrLen(nameKey) - 1);
        }
        
        BT_RecordData[i][RDATA_TIME]    = recordTime;
        BT_RecordData[i][RDATA_NAME]    = recordName;
        BT_RecordData[i][RDATA_DATE]    = recordDate;
        BT_RecordData[i][RDATA_ACCOUNT] = recordAccount;
    }
    
    FreeDBResults(sortedTimes);
    
    // clear out empty records just in case
    for (int j = i; j < RECORDCOUNT; j++)
    {
        BT_RecordData[j][RDATA_TIME]    = -1;
        BT_RecordData[j][RDATA_NAME]    = "";
        BT_RecordData[j][RDATA_DATE]    = "";
        BT_RecordData[j][RDATA_ACCOUNT] = "";
    }
}


function int BTimer_GetRecord(str name)
{
    str table_times = BTimer_DBColumnName(RDATA_TIME);
    
    str cleanName = strLower(cleanString(name));
    int curRecord = GetDBEntry(table_times, cleanName);
    
    if (curRecord > 0) { return curRecord; }
    return -1;
}


function int BTimer_UpdateRecord(int pln, int time)
{
    // either you're cheating or a bug occured, either way *no*
    if (time <= 0) { return false; }
    
    // sanity check
    if (pln < 0 || pln >= PLAYERMAX) { return false; }
    
    str name      = StrParam(n:pln+1);
    str account   = GetPlayerAccountName(pln);
    str nameKey   = strLower(cleanString(name));
    
    // character 30: ASCII record separator, cannot appear in player names
    // non-logged in players have an account name "<pln>@localhost"
    // account names can't have @ in them
    
    if (!stringBlank(account) && (strstr(account, "@") < 0))
    {
        nameKey = StrParam(c:30, s:account);
    }
    
    str table_times = BTimer_DBColumnName(RDATA_TIME);
    str table_dates = BTimer_DBColumnName(RDATA_DATE);
    str table_names = BTimer_DBColumnName(RDATA_NAME);
    
    int curRecord = GetDBEntry(table_times, nameKey);

    if ((curRecord <= 0) || (curRecord > time))
    {
        BeginDBTransaction();
        SetDBEntry(table_times, nameKey, time);
        SetDBEntryString(table_dates, nameKey, Strftime(SystemTime(), "%Y-%m-%d %H:%M:%S %z"));
        SetDBEntryString(table_names, nameKey, name);
        EndDBTransaction();
        
        return true;
    }
    
    return false;
}



script "BTimer_Open" OPEN
{
    int lastUpdate = -1;
    int showedTime = false;
    
    BTimer_LoadRecords();
    
    SetHudSize(800, 600, true);
    
    while (true)
    {
        int t = Timer();
        
        if (!showedTime || (t / 35) > ((t - 1) / 35))
        {
            showedTime = true;
            
            SetFont("DBIGFON2");
            HudMessageBold(s:timeString(t, false);
                HUDMSG_PLAIN | HUDMSG_COLORSTRING, 198, "Timer_Global", 20.1, 318.0, 0);
        }
        
        if (t < 4 || t % 35 == 1)
        {
            SetFont("SMALLFONT");
            SetHudSize(640, 480, true);
            HudMessage(s:"Records";
                HUDMSG_PLAIN | HUDMSG_COLORSTRING, 199, "Lime", 20.1, 286.0, 0);
            SetHudSize(800, 600, true);
            
            for (int i = 0; i < RECORDCOUNT; i++)
            {
                int recordTime    = BT_RecordData[i][RDATA_TIME];
                int idOffset      = i*5;
                
                if (recordTime <= 0)
                {
                    HudMessage(s:""; HUDMSG_PLAIN, 201 + idOffset, 0,0,0,0);
                    HudMessage(s:""; HUDMSG_PLAIN, 202 + idOffset, 0,0,0,0);
                    HudMessage(s:""; HUDMSG_PLAIN, 203 + idOffset, 0,0,0,0);
                    HudMessage(s:""; HUDMSG_PLAIN, 204 + idOffset, 0,0,0,0);
                    HudMessage(s:""; HUDMSG_PLAIN, 205 + idOffset, 0,0,0,0);
                    continue;
                }
                
                str recordName    = BT_RecordData[i][RDATA_NAME];
                str recordAccount = BT_RecordData[i][RDATA_ACCOUNT];
                
                HudMessage(s:"#", d:i+1, s:":";
                    HUDMSG_PLAIN | HUDMSG_COLORSTRING, 201 + idOffset, BT_RecordColors[i], 20.1, 373.0 + (i * 23.0), 0);
                
                HudMessage(s:StrParam(s:"Time: \c[gray]", s:timeString(recordTime, true));
                    HUDMSG_PLAIN | HUDMSG_COLORSTRING, 202 + idOffset, BT_RecordColors[i], 20.1, 382.0 + (i * 23.0), 0);
                    
                if (!stringBlank(recordAccount))
                {  
                    HudMessage(s:"*";
                        HUDMSG_PLAIN | HUDMSG_COLORSTRING, 205 + idOffset, "Lime", 18.2, 378.0 + (i * 23.0), 0);
                    
                    int second = t / 35;
                    
                    switch (second % 15)
                    {
                      default:
                        HudMessage(s:recordName;
                            HUDMSG_PLAIN | HUDMSG_COLORSTRING,  203 + idOffset, BT_RecordColors[i], 43.1, 373.0 + (i * 23.0), 0);
                            
                        HudMessage(s:""; HUDMSG_PLAIN, 204 + idOffset, 0,0,0,0);
                        break;
                      
                      case 10:
                        HudMessage(s:recordName;
                            HUDMSG_FADEOUT | HUDMSG_COLORSTRING, 203 + idOffset, BT_RecordColors[i], 43.1, 373.0 + (i * 23.0), 0, 1.0);
                            
                        SetFont("STATUSRP");
                        HudMessage(s:recordAccount;
                            HUDMSG_FADEINOUT | HUDMSG_COLORSTRING, 204 + idOffset, BT_RecordColors[i], 43.1, 373.0 + (i * 23.0), 1.0, 1.0, 1.0);
                        SetFont("SMALLFONT");
                        break;
                      
                      case 11:
                      case 12:
                      case 13:
                        HudMessage(s:""; HUDMSG_PLAIN, 203 + idOffset, 0,0,0,0);
                        
                        SetFont("STATUSRP");
                        HudMessage(s:recordAccount;
                            HUDMSG_PLAIN | HUDMSG_COLORSTRING, 204 + idOffset, BT_RecordColors[i], 43.1, 373.0 + (i * 23.0), 0);
                        SetFont("SMALLFONT");
                        break;
                      
                      case 14:
                        HudMessage(s:recordName;
                            HUDMSG_FADEINOUT | HUDMSG_COLORSTRING, 204 + idOffset, BT_RecordColors[i], 43.1, 373.0 + (i * 23.0), 1.0, 1.0, 1.0);
                            
                        SetFont("STATUSRP");
                        HudMessage(s:recordAccount;
                            HUDMSG_FADEOUT | HUDMSG_COLORSTRING, 203 + idOffset, BT_RecordColors[i], 43.1, 373.0 + (i * 23.0), 0, 1.0);
                        SetFont("SMALLFONT");
                        break;
                    }
                      
                }
                else
                {
                    HudMessage(s:recordName;
                        HUDMSG_PLAIN | HUDMSG_COLORSTRING, 203 + idOffset, BT_RecordColors[i], 43.1, 373.0 + (i * 23.0), 0);
                        
                    HudMessage(s:""; HUDMSG_PLAIN, 204 + idOffset, 0,0,0,0);
                    HudMessage(s:""; HUDMSG_PLAIN, 205 + idOffset, 0,0,0,0);
                }
                    
            }
        }
        
        Delay(1);
    }
}



script "BTimer_Display" ENTER
{
    int showedTime   = false;
    int pln = PlayerNumber();
    
    SetHudSize(800, 600, true);
    SetFont("DBIGFON2");
    
    //display timer stuff
    while (true)
    {
        int t               = Timer();
        int finishTime      = BMaps_FinishTimes[pln];
        int timeSinceFinish = cond(finishTime > 0, t - finishTime, -1);

        if (!showedTime || (t % 35) == 0 || (timeSinceFinish > 0 && timeSinceFinish <= 35))
        {
            showedTime = true;
            
            if (finishTime > 0)
            {
                str timestr = timeString(finishTime, true);
                HudMessage(s:timestr; HUDMSG_FADEOUT | HUDMSG_COLORSTRING, 300, "Timer_Personal", 20.1, 335.0, 1.25, 0.5);
                
                if (timeSinceFinish <= 35)
                {
                    int a = 1.0 - (itof(timeSinceFinish) / 35);
                    HudMessage(s:timestr; HUDMSG_FADEOUT | HUDMSG_COLORSTRING| HUDMSG_ALPHA, 299, "NewGold", 20.1, 335.0, 0, a, a);
                }
            }
            else
            {
                HudMessage(s:timeString(t, false);
                    HUDMSG_FADEOUT | HUDMSG_COLORSTRING, 300, "Timer_Personal", 20.1, 335.0, 1.25, 0.5);
            }
        }
        
        Delay(1);
    }
}



script "BTimer_Finish" (void)
{
    if (CheckInventory("ShouldBeGhost")) { terminate; }
    
    int pln = PlayerNumber();
    if (pln < 0 || BMaps_FinishTimes[pln] > 0) { terminate; }
    
    int playerTime         = Timer();
    BMaps_FinishTimes[pln] = playerTime;
    
    if (BTimer_UpdateRecord(pln, playerTime))
    {
        BTimer_LoadRecords();
    }
}



//----------------
//map exit scripts
//----------------



script "BMaps_Exit" (int countdown)
{
    int pln = PlayerNumber();
    
    if (CheckInventory("ShouldBeGhost") || pln < 0)
    {
        SetResultValue(false);
        terminate;
    }
    
    SetResultValue(true);
    
    //if the mapper hasn't put a trigger in, make sure a timer is registered
    ACS_NamedExecuteWithResult("BTimer_Finish");
    
    if (BState_PlayersLeft() > 0)
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
    countdown = cond(countdown > 0, countdown, 30);
    
    SetHudSize(800, 600, true);
    SetFont("BIGFONT");
    
    SetActivator(0);
    
    for (int i = 0; i < countdown; i++)
    {
        if (BState_PlayersLeft() <= 0) { break; }
        
        HudMessageBold(s:"Map ends in ", d:countdown, s:"...";
            HUDMSG_PLAIN, 197, CR_RED, 400.0, 100.0, 0);
        
        Delay(35);
    }
    
    Exit_Normal(0);
}