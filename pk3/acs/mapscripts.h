script "BMaps_IntroCard" (int title, int author, int difficulty, int dumbquote) clientside
{
    if (GetCVar("bmaps_cl_notitlecard") || !GetCVar("survival") || GetGameModeState() == GAMESTATE_COUNTDOWN)
    {
        terminate;
    }
    
    Delay(18);
    SetHudSize(640, 480, true);
    SetHudClipRect(160, 0, 320, 480, 320, false);
    
    int hasQuote        = !stringBlank(dumbquote);
    int fullBrightIters = cond(hasQuote, 16, 12);
    int fadeIters       = 8;
    int useFireblu      = GetCVar("bmaps_cl_shitpost");
    
    str firebluColor, titleColor, authorColor, difficultyColor, quoteColor;
    
    if (useFireblu)
    {
        title      = cleanString(title);
        author     = cleanString(author);
        difficulty = cleanString(difficulty);
        dumbquote  = cleanString(dumbquote);
    }
    else
    {
        titleColor      = "Orange";
        authorColor     = "Yellow";
        difficultyColor = "Gold";
        quoteColor      = "White";
    }
    
    int i;
    
    for (i = 0; i < fullBrightIters; i++)
    {
        if (useFireblu)
        {
            firebluColor    = cond(i % 2, "FIREBLU", "FIREBLU2");
            titleColor      = firebluColor;
            authorColor     = firebluColor;
            difficultyColor = firebluColor;
            quoteColor      = firebluColor;
        }
        
        SetFont("BIGFONT");
        HudMessage(s:title;
                    HUDMSG_PLAIN | HUDMSG_COLORSTRING, -500, titleColor, 320.4, 340.0, 1.0);
        
        if (i >= 3)
        {
            SetFont("SMALLFONT");
            HudMessage(s:"by ", s:author;
                    HUDMSG_PLAIN | HUDMSG_COLORSTRING, -501, authorColor, 320.4, 355.0, 1.0);
        }
        
        if (i >= 7)
        {
            SetFont("SMALLFONT");
            HudMessage(s:"Difficulty: ", s:difficulty;
                    HUDMSG_PLAIN | HUDMSG_COLORSTRING, -502, difficultyColor, 320.4, 367.0, 1.0);
        }
        
        if (i >= 11 && hasQuote)
        {
            SetFont("SMALLFONT");
            HudMessage(s:"\"", s:dumbquote, s:"\"";
                    HUDMSG_PLAIN | HUDMSG_COLORSTRING, -503, quoteColor, 320.4, 378.1, 1.0);
        }
        
        Delay(8);
    }
    
    int alpha, fadeTime;
    
    for (i = 0; i < fadeIters; i++)
    {
        if (useFireblu)
        {
            firebluColor    = cond((i + fullBrightIters) % 2, "FIREBLU", "FIREBLU2");
            titleColor      = firebluColor;
            authorColor     = firebluColor;
            difficultyColor = firebluColor;
            quoteColor      = firebluColor;
        }
        
        alpha    = 1.0 - ((1.0 / fadeIters) * i);
        fadeTime = 0.25 * (fadeIters - i);
        
        SetFont("BIGFONT");
        HudMessage(s:title;
                    HUDMSG_FADEOUT | HUDMSG_ALPHA | HUDMSG_COLORSTRING, -500, titleColor, 320.4, 340.0, 0, fadeTime, alpha);
        
        SetFont("SMALLFONT");
        HudMessage(s:"by ", s:author;
                    HUDMSG_FADEOUT | HUDMSG_ALPHA | HUDMSG_COLORSTRING, -501, authorColor, 320.4, 355.0, 0, fadeTime, alpha);
                    
        SetFont("SMALLFONT");
        HudMessage(s:"Difficulty: ", s:difficulty;
                    HUDMSG_FADEOUT | HUDMSG_ALPHA | HUDMSG_COLORSTRING, -502, difficultyColor, 320.4, 367.0, 0, fadeTime, alpha);
        
        if (hasQuote)
        {
            SetFont("SMALLFONT");
            HudMessage(s:"\"", s:dumbquote, s:"\"";
                    HUDMSG_FADEOUT | HUDMSG_ALPHA | HUDMSG_COLORSTRING, -503, quoteColor, 320.4, 378.1, 0, fadeTime, alpha);
        }
        
        Delay(8);
    }
}