#pragma noroot

#include <types.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <control.h>
#include <lineedit.h>
#include <locator.h>
#include <memory.h>
#include <menu.h>
#include <misctool.h>
#include <orca.h>
#include <qdaux.h>
#include <quickdraw.h>
#include <resources.h>
#include <window.h>

#include "babelfish.h"

#include "opts.h"

#define WINDOW 0x00000FFA
#define VERSION 0x00000002
#define CANCEL 0x00000009
#define ACCEPT 0x0000000A
#define FONTMENU 0x0000000C
#define PAGEMENU 0x00000015
#define SPACEMENU 0x00000016
#define TOPMENU 0x0000000F
#define LEFTMENU 0x00000010
#define RIGHTMENU 0x00000011
#define BOTTOMMENU 0x00000012


typedef struct fontItem {
    word            famNum;
    word            itemID;
    char            name[27];
    struct fontItem **next;
} fontItem, *fontItemPtr, **fontItemHndl;

typedef struct _version {
    long version;
    word country;
    char pName[14];
    //copyright info after name
} version, *versionPtr, **versionHndl;

//forward declarations
void DrawWindow(void);
bool validateFontSize(WindowPtr wind, long controlID);
void setCtls(WindowPtr wind);

word userID;

#pragma databank 1
void exportOpt(BFXferRecPtr xfer) {
    WindowPtr     myWindow;
    EventRecord   CurrentEvent;
    word          controlID = 0;
    CtlRecHndl ctl;
    versionHndl   ver;
    bool done = false;
    char verStr[10] = { 0 };

    userID = MMStartUp();

    //load the options
    loadOptions();

    myWindow = NewWindow2(NULL, 0, &DrawWindow, NULL, refIsResource, WINDOW, rWindParam1);
    SetPort(myWindow);

    //version
    ver = (versionHndl)LoadResource(rVersion, 1);
    VersionString(0, (*ver)->version, verStr);
    ctl = GetCtlHandleFromID(myWindow, VERSION);
    (*ctl)->ctlMoreFlags = 0x1000;
    memmove(verStr + 2, verStr + 1, 8);
    verStr[1] = 'v';
    (*ctl)->ctlData = (long)(verStr + 1);
    (*ctl)->ctlValue = strlen(verStr + 1);

    //initialize the values
    SetCtlValueByID(getDefaultFont(), myWindow, FONTMENU);
    SetCtlValueByID(getPageSize(), myWindow, PAGEMENU);
    SetCtlValueByID(getSpacing(), myWindow, SPACEMENU);
    SetCtlValueByID(getTopMargin(), myWindow, TOPMENU);
    SetCtlValueByID(getLeftMargin(), myWindow, LEFTMENU);
    SetCtlValueByID(getRightMargin(), myWindow, RIGHTMENU);
    SetCtlValueByID(getBottomMargin(), myWindow, BOTTOMMENU);
 
    ShowWindow(myWindow);
    InitCursor();

    done = false;
    while (!done) {
        controlID = DoModalWindow(&CurrentEvent, NULL,
                                  (VoidProcPtr)0x80000000L, NULL, 0x0);

        switch (controlID) {
        case ACCEPT:
        case CANCEL:
            done = true;
            break;
        default:
            break;
        }
    }

    if (controlID == ACCEPT) {
        setDefaultFont(GetCtlValueByID(myWindow, FONTMENU));
        setPageSize(GetCtlValueByID(myWindow, PAGEMENU));
        setSpacing(GetCtlValueByID(myWindow, SPACEMENU));
        setTopMargin(GetCtlValueByID(myWindow, TOPMENU));
        setLeftMargin(GetCtlValueByID(myWindow, LEFTMENU));
        setRightMargin(GetCtlValueByID(myWindow, RIGHTMENU));
        setBottomMargin(GetCtlValueByID(myWindow, BOTTOMMENU));
        saveOptions();
    }

    CloseWindow(myWindow);
}
#pragma databank 0

void DrawWindow(void) {
    DrawControls(GetPort());
}


