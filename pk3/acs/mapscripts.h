script "BMaps_IntroCard" (int title, int author, int difficulty, int dumbquote) clientside
{
    if (GetCVar("bmaps_cl_notitlecard") || !GetCVar("survival") || GetGameModeState() == GAMESTATE_COUNTDOWN)
    {
        terminate;
    }
    
    Delay(18);
    SetHudSize(640, 480, true);
    SetHudClipRect(160, 0, 320, 480, 320, false);
    
    SetFont("BIGFONT");
    HudMessage(s:title;
                HUDMSG_FADEOUT, -500, CR_ORANGE, 320.4, 340.0, 3.0, 1.0);
                
    SetFont("SMALLFONT");
    HudMessage(s:"by ", s:author;
                HUDMSG_FADEOUT, -501, CR_YELLOW, 320.4, 355.0, 3.0, 1.0);
    
    Delay(35);
    
    
    // I don't want to bother with figuring out the timing here, so I'm just gonna print it again
    SetFont("BIGFONT");
    HudMessage(s:title;
                HUDMSG_FADEOUT, -500, CR_ORANGE, 320.4, 340.0, 3.0, 1.0);
                
    SetFont("SMALLFONT");
    HudMessage(s:"by ", s:author;
                HUDMSG_FADEOUT, -501, CR_YELLOW, 320.4, 355.0, 3.0, 1.0);
    
    HudMessage(s:"Difficulty: ", s:difficulty;
                HUDMSG_FADEOUT, -502, CR_GOLD, 320.4, 370.0, 3.0, 1.0);
    Delay(35);
    
    if (!stringBlank(dumbquote))
    {
        SetFont("BIGFONT");
        HudMessage(s:title;
                    HUDMSG_FADEOUT, -500, CR_ORANGE, 320.4, 340.0, 2.0, 1.0);
                    
        SetFont("SMALLFONT");
        HudMessage(s:"by ", s:author;
                    HUDMSG_FADEOUT, -501, CR_YELLOW, 320.4, 355.0, 2.0, 1.0);
        
        HudMessage(s:"Difficulty: ", s:difficulty;
                    HUDMSG_FADEOUT, -502, CR_GOLD, 320.4, 370.0, 2.0, 1.0);
        
        int quoteTime = min(6.5, 2.5 + (0.02 * StrLen(dumbquote)));
        
        HudMessage(s:"\"", s:dumbquote, s:"\"";
                    HUDMSG_FADEOUT, -503, CR_WHITE, 320.4, 378.1, quoteTime, 1.0);
    }
}