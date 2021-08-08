#pragma noroot

#include <types.h>
#include <string.h>

#include <font.h>
#include <locator.h>
#include <memory.h>

#include "babelfish.h"

#include "opts.h"

extern word userID;
static opts options = { PREF_VERSION, 1, 1, 1, 4, 4, 4, 4 };

void loadOptions(void) {
    BFLoadDataIn loadDataIn;
    BFLoadDataOut loadDataOut = { 0 };

    loadDataIn.userID = userID;
    SendRequest(BFLoadData,
                sendToName + stopAfterOne,
                (long)NAME_OF_BABELFISH,
                (long)&loadDataIn,
                (ptr)&loadDataOut);

    if (loadDataOut.bfResult == bfNoErr) {
        memcpy(&options, *(loadDataOut.dataHandle), sizeof(opts));
        DisposeHandle((Handle)loadDataOut.dataHandle);
    }
}

void saveOptions(void) {
    BFSaveDataIn saveDataIn;
    BFSaveDataOut saveDataOut;

    saveDataIn.userID = userID;
    saveDataIn.dataHandle = NewHandle(sizeof(opts), userID, 0, NULL);
    PtrToHand((ptr)&options, saveDataIn.dataHandle, sizeof(opts));
    SendRequest(BFSaveData,
                sendToName + stopAfterOne,
                (long)NAME_OF_BABELFISH,
                (long)&saveDataIn,
                (ptr)&saveDataOut);
}

word getDefaultFont(void) {
    return options.defFont;
}

void setDefaultFont(word id) {
    options.defFont = id;
}

word getPageSize(void) {
    return options.pageSize;
}

void setPageSize(word size) {
    options.pageSize = size;
}

word getSpacing(void) {
    return options.spacing;
}

void setSpacing(word size) {
    options.spacing = size;
}

word getTopMargin(void) {
    return options.topMargin;
}

void setTopMargin(word margin) {
    options.topMargin = margin;
}

word getLeftMargin(void) {
    return options.leftMargin;
}

void setLeftMargin(word margin) {
    options.leftMargin = margin;
}

word getRightMargin(void) {
    return options.rightMargin;
}

void setRightMargin(word margin) {
    options.rightMargin = margin;
}

word getBottomMargin(void) {
    return options.bottomMargin;
}

void setBottomMargin(word margin) {
    options.bottomMargin = margin;
}

