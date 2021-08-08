#pragma noroot

#include <types.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#include <memory.h>
#include <locator.h>
#include <font.h>
#include <gsos.h>
#include <orca.h>
#include <intmath.h>
#include <stdarg.h>

#include "babelfish.h"
#include "export.h"

extern word userID;

word dispatch(word request, BFXferRecHndl dataIn, BFResultOutPtr dataOut) {
    int result = 0;

    switch (request) {
    case srqGoAway:      
        ((srqGoAwayOutPtr)dataOut)->resultID = MMStartUp();
        result = 0x8000;
        break;
    case TrStartUp:
        userID = MMStartUp();
        exportStartup(*dataIn, dataOut);
        result = 0x8000;
        break;
    case TrShutDown:
         exportCleanUp(*dataIn, dataOut);
        result = 0x8000;
        break;
    case TrWrite:
        trWrite(*dataIn, dataOut);
        result = 0x8000;
        break;
    }
    return result;
}

