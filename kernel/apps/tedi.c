#include "tedi.h"

void printChar(char printData, uint16* curX, uint16* curY)
{
    if(printData == '\t')
    {
        uint8 insCount = *curX % 4;
        insCount = insCount == 0 ? 4 : insCount;
        while(insCount-- > 0)
        {
            // The brackets are meant to be there. DO NOT DELETE
            printCharAt(' ', black, (*curX)++ % sw, *curY);
        }
    }
    else if(printData == '\n')
    {
        // The bracket thing:
        // *ptr++ increments pointer -> derefernces (shorthand for ptr++; *ptr)
        // (*ptr)++ derefernces pointer -> increments (shorthand for *ptr += 1)
        (*curY)++; *curX = 0;
    }
    else
    {
        printCharAt(printData, black, (*curX)++ % sw, *curY);
    }
}

void appendln(strbuilder_t* data, uint16* curX, uint16* curY, uint32* index)
{
    (*curY)++; *curX = 0;
    strbuilder_insertc(data, '\n', (*index)++);
}

string tedi_session()
{
    string vidmem = (string) 0xb8000;
    char oldmem[strlen(vidmem)];
    strcpy(oldmem, vidmem);
    paintScreen(blue);

    bool inCmdMode = true, shiftDown = false;
    uint16 curX = 0, curY = 0;
    uint32 index = 0;
    static int cmdKeys[] = {0x10 /*Q*/, 0x17 /*I*/, 0x18 /*O*/};
    strbuilder_t data = strbuilder_init();
    int k;
    while(true)
    {
        cursorX = curX % sw;
        cursorY = curY;
        updateCursor();
        // The trailing spaces clears out junky characters! Keep them
        printAt(inCmdMode ? "CMD     " : "INS     ", black, 2, 24);
        printAt(itos10(curX), black, 6, 24);
        printAt(":     ", black, 9, 24);
        printAt(itos10(curY), black, 11, 24);
        if(inCmdMode)
        {
            k = waitUntilKey(cmdKeys);
            switch(k)
            {
            case 0x10:
                goto end;
            case 0x17:
                inCmdMode = false;
                break;
            case 0x18:
                appendln(&data, &curX, &curY, &index);
                inCmdMode = false;
                break;
            }
        } else {
            k = getAnyKey();
            char charInput = ' ';
            switch(k)
            {
            case 0x01:
                inCmdMode = true;
                break;
            case 0x2A:
            case 0x36:
                shiftDown = true;
                break;
            case 0xAA:
            case 0xB6:
                shiftDown = false;
                break;
            case 0x1C:
                appendln(&data, &curX, &curY, &index);
                break;
            case 0x48:
                curY--;
                break;
            case 0x4B:
                curX--;
                index--;
                break;
            case 0x4D:
                curX++;
                index++;
                break;
            case 0x50:
                curY++;
                break;
            case 0x0E:
            {
                strbuilder_rmchar(&data, index);
                // Hahaha... RE-PRINT THE ENTIRE STRB!!!
                // Its all warped strings fault... :(
                curX = 0; curY = 0;
                paintLine(blue, 0, curY, sw);
                for(uint16 loopi = 0; loopi < data.ilist.size; loopi++) {
                    printChar(strbuilder_charAt(data, loopi), &curX, &curY);
                }
                index--;
                break;
            }
            default:
                if(k < 59 && k > 0)
                {
                    if(shiftDown)
                    {
                        charInput = kbShiftChars[k];
                    }
                    else
                    {
                        charInput = kbLowerChars[k];
                    }
                    if(charInput == 0)
                    {
                        break;
                    }
                    strbuilder_insertc(&data, charInput, index++);
                    // Hahaha... RE-PRINT THE ENTIRE STRB!!!
                    curX = 0; curY = 0;
                    uint16 oldX = 0, oldY = 0, indexClone = index;
                    paintLine(blue, 0, curY, sw);
                    for(uint16 loopi = 0; loopi < data.ilist.size; loopi++) {
                        printChar(strbuilder_charAt(data, loopi), &curX, &curY);
                        if(curY > oldY)
                        {
                            indexClone -= oldX + 1;
                        }
                        oldY = curY;
                        oldX = curX;
                    }
                    curX = indexClone;
                }
                break;
            }
        }
    }
end: // Sorry for the mom spaghetti code
    // Must be last line (before any prints)
    strcpy(vidmem, oldmem);
    string msg = strbuilder_tostr(data);
    strbuilder_destroy(&data);
    cursorX = cursorY = 0;
    print(msg, black);
    return msg;
}
