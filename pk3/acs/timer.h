//ingame timer, and storing of server records

int recordTimes[5]; //seconds
int recordNames[5];
int recordDates[5];

//will replace these with custom colours later
int recordColors[5] = {CR_GOLD, CR_WHITE, CR_TAN, CR_GRAY, CR_DARKGRAY};

int finished[64][2];


script "BTimer_Open" OPEN
{
	BTimer_LoadRecords();
	while(1)
	{
		int seconds = Timer() / 35;
		int minutes = seconds / 60;
		seconds = seconds % 60;
		
		int singleDigit = "";
		if(seconds < 10) singleDigit = "0";
		
		SetHudSize(800, 600, 0);
		SetFont("BIGFONT");
		
		HudMessageBold(d:minutes, s:":", s:singleDigit, d:seconds;
		HUDMSG_PLAIN, 198, CR_DARKGREEN, 20.5, 565.0, 0);
		
		Delay(35);
	}
}

script "BTimer_Display" ENTER
{
	//display timer stuff
	while(1)
	{
		int seconds = Timer() / 35;
		if(finished[PlayerNumber()][0]) seconds = finished[PlayerNumber()][1];
		int minutes = seconds / 60;
		seconds = seconds % 60;
		
		int singleDigit = "";
		if(seconds < 10) singleDigit = "0";
		
		SetHudSize(800, 600, 0);
		SetFont("BIGFONT");
		//display current time
		HudMessage(d:minutes, s:":", s:singleDigit, d:seconds;
		HUDMSG_PLAIN, 200, CR_GREEN, 20.5, 550.0, 0);
		
		//display records
		SetFont("SMALLFONT");
		HudMessage(s:"Server records";
				HUDMSG_PLAIN, 199, CR_DARKGREEN, 20.5, 480.0, 0);
		for(int i = 0; i < 5; i++)
		{
			if(recordTimes[i] > 0)
			{
				int newSeconds = recordTimes[i];
				int newMinutes = newSeconds / 60;
				newSeconds = newSeconds % 60;
				
				int newSingleDigit = "";
				if(newSeconds < 10) newSingleDigit = "0";
				HudMessage(s:"#", d:(i+1), s:"- ", d:newMinutes, s:":", s:newSingleDigit, d:newSeconds, s:" by ", s:recordNames[i];
				HUDMSG_PLAIN, 201 + i, recordColors[i], 20.5, 490.0 + i * 10.0, 0);
			}
		}
		
		Delay(35);
	}
}

script "BTimer_Finish" (void)
{
	int pID = PlayerNumber();
	
	if(CheckInventory("ShouldBeGhost")) Terminate;
	if(pID == -1) Terminate;
	if(finished[pID][0]) Terminate;
	
	finished[pID][0] = 1;
	
	int myTime = Timer() / 35;
	finished[pID][1] = myTime;
	
	int newPos = 0;
	
	for(int i = 0; i < 5; i++)
	{
		if(myTime < recordTimes[i] || recordTimes[i] == 0)
		{
			newPos = i + 1;
			break;
		}
	}
	
	if(newPos)
	{
		int myName = strLower(cleanString(StrParam(n:pID)));
		BeginDBTransaction();
		//do you have a currently existing record?
		int existRecord = 0;
		for(i = 0; i < 5; i++)
		{
			if(recordNames[i] == myName)
			{
				existRecord = i + 1;
				break;
			}
		}
		
		if(existRecord)
		{
			//time was worse or equal, scrap
			if(existRecord < newPos) {}
			
			//time was better than old, but still same position- replace old
			else if(newPos == existRecord)
			{
				SetDBEntry(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_times"), existRecord, myTime);
				SetDBEntryString(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_names"), existRecord, myName);
				SetDBEntryString(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_dates"), existRecord, Strftime(SystemTime(), "%D", true));
			}
			else
			{
				//time was better and moved up- swap old with worse times
				//move all other entries down
				for(i = newPos - 1; i < existRecord; i++)
				{
					SetDBEntry(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_times"), i+2, recordTimes[i]);
					SetDBEntryString(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_names"), i+2, recordNames[i]);
					SetDBEntryString(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_dates"), i+2, recordDates[i]);
				}
				
				SetDBEntry(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_times"), newPos, myTime);
				SetDBEntryString(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_names"), newPos, myName);
				SetDBEntryString(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_dates"), newPos, Strftime(SystemTime(), "%D", true));
			}
		}
		
		else
		{
			//move all other entries down
			for(i = newPos - 1; i < 4; i++)
			{
				SetDBEntry(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_times"), i+2, recordTimes[i]);
				SetDBEntryString(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_names"), i+2, recordNames[i]);
				SetDBEntryString(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_dates"), i+2, recordDates[i]);
			}
			
			SetDBEntry(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_times"), newPos, myTime);
			SetDBEntryString(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_names"), newPos, myName);
			SetDBEntryString(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_dates"), newPos, Strftime(SystemTime(), "%D", true));
		}
		
		BTimer_LoadRecords();
		EndDBTransaction();
	}
}

function void BTimer_LoadRecords(void)
{
	//grab existing records
	for(int i = 0; i < 5; i++)
	{
		recordTimes[i] = GetDBEntry(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_times"), i+1);
		recordNames[i] = GetDBEntryString(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_names"), i+1);
		recordDates[i] = GetDBEntryString(StrParam(s:"bmaps_", n:PRINTNAME_LEVEL, s:"_dates"), i+1);
	}
}

//----------------
//map exit scripts
//----------------

script "BMaps_Exit" (int countdown)
{
	if(CheckInventory("ShouldBeGhost")) Terminate;
	if(PlayerNumber() == -1) Terminate;
	
	if(!PlayersLeft()) Exit_Normal(0);
	else ACS_NamedExecute("BMaps_Exit_Countdown", 0, countdown, 0, 0);
}

script "BMaps_Exit_Countdown" (int countdown)
{
	SetHudSize(800, 600, 0);
	SetFont("BIGFONT");
	while(countdown)
	{
		HudMessageBold(s:"Map ends in ", d:countdown, s:"...";
		HUDMSG_PLAIN, 197, CR_RED, 400.0, 100.0, 0);
		countdown--;
		
		if(!PlayersLeft()) countdown = 0;
		
		Delay(35);
	}
	Exit_Normal(0);
}

function int PlayersLeft (void)
{
	int players = 0;
	int finish = 0;
	
	for(int i = 0; i < 64; i++)
	{
		if(PlayerInGame(i) && !CheckActorInventory(
			ACS_NamedExecuteWithResult("BMaps_GetPlayerTID", i, 0, 0, 0), "ShouldBeGhost")) players++;
		
		if(finished[i][0]) finish++;
	}
	
	return (players - finish);
}