#pragma noroot

#include <types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdbool.h>

#include <memory.h>
#include <locator.h>
#include <font.h>
#include <gsos.h>
#include <orca.h>
#include <intmath.h>
#include <event.h>
#include <window.h>
#include <quickdraw.h>
#include <resources.h>
#include <control.h>
#include <misctool.h>

#include "babelfish.h"
#include "pdfgen.h"
#include "opts.h"
#include "export.h"

#define SIS_FONT_3 0x9004
#define SIS_FONT_4 0x9005
#define SIS_FONT_5 0x9006

extern word userID;
extern pascal void SystemUserID(unsigned, char *);
extern pascal void SystemEnvironmentInit(void);
extern pascal void SystemSANEInit(void);
extern pascal void SystemSANEShutDown(void);
extern const uint16_t* find_font_widths(const char *font_name);
extern int pdf_text_point_width(struct pdf_doc *pdf, const char *text,
                                ptrdiff_t text_len, float size,
                                const uint16_t *widths, float *point_width);

char *fontTable[][4] = {
    { "Helvetica-BoldOblique", "Helvetica-Bold", "Helvetica-Oblique", "Helvetica" },
    { "Courier-BoldOblique", "Courier-Bold", "Courier-Oblique", "Courier" },
    { "Times-BoldItalic", "Times-Bold", "Times-Italic", "Times-Roman" },
    { "Symbol", "Symbol", "Symbol", "Symbol" },
    { "ZapfDingbats", "ZapfDingbats", "ZapfDingbats", "ZapfDingbats" },
    };

float pageDimensions[][2] = {
    { PDF_LETTER_WIDTH, PDF_LETTER_HEIGHT },
    { PDF_A4_WIDTH, PDF_A4_HEIGHT },
    { PDF_A3_WIDTH, PDF_A3_HEIGHT }
};

float spacings[] = { 0, 1.20F, 1.45f };

float margins[] = {
    0, PDF_INCH_TO_POINT(0.25f), PDF_INCH_TO_POINT(0.50f), PDF_INCH_TO_POINT(0.75f),
    PDF_INCH_TO_POINT(1.00f), PDF_INCH_TO_POINT(1.25f), PDF_INCH_TO_POINT(1.50f)
};

typedef struct {
    char colorName[15];
    word id;
    uint32_t rgb;
} colorList;

colorList colors[] = {
    { "black", 0x0000, 0x00000 },
    { "blue", 0x1111, 0x0000FF },
    { "darkgreen", 0x2222, 0x006400 },
    { "gray", 0x3333, 0xa9a9a9 },
    { "red", 0x4444, 0xff0000 },
    { "purple", 0x5555, 0x800080 },
    { "orange", 0x6666, 0xffa500 },
    { "fuchsia", 0x7777, 0xFF00FF },
    { "green", 0x8888, 0x008000 },
    { "cyan", 0x9999, 0x00FFFF },
    { "lightgreen", 0xAAAA, 0x00FF00 },
    { "teal", 0xBBBB, 0x008080 },
    { "silver", 0xCCCC, 0xD3D3D3 },
    { "cornflowerblue", 0xDDDD, 0x6495ed },
    { "yellow", 0xEEEE, 0xFFFF00 },
    { "white", 0xFFFF, 0xFFFFFF },
    { NULL, 0, 0 }
};

Handle newData = NULL;
static GSString255 exportFile;
struct pdf_doc *pdf;
static float x = 0;
static float y = 0;
static const char *lastFontFace = NULL;
static float maxSize = 0;
static float lineWidth = PDF_LETTER_WIDTH;
static float lastMaxSize = 0;
static char outputFilePath[255] = { 0 };

word writeData(word refNum, char *data) {
    IORecGS       writeRec = { 4 };

    writeRec.refNum = refNum;
    writeRec.dataBuffer = data;
    writeRec.requestCount = strlen(data);
    WriteGS(&writeRec);

    return toolerror();
}

void resetX(void) {
    x = margins[getLeftMargin()];
}

void resetY(void) {
    y = pageDimensions[getPageSize()-1][1] - margins[getTopMargin()];
}

void exportStartup(BFXferRecPtr dataIn, BFResultOutPtr dataOut) {
    struct pdf_info info = {
        "Babelfish", "PDF Writer", "", "", "", ""
    };
    TimeRec time = { 0 };

    SystemUserID(userID, NULL);
    SystemEnvironmentInit();
    SystemSANEInit();

    strncpy(outputFilePath, dataIn->filePathPtr->text, dataIn->filePathPtr->length);

    time = ReadTimeHex();
    sprintf(info.date, "%02d/%02d/20%02d %02d:%02d:%02d",
            time.month, time.day, time.year, time.hour, time.minute, time.second);

    pdf = pdf_create(pageDimensions[getPageSize()-1][0], pageDimensions[getPageSize()-1][1], &info);
    pdf_set_font(pdf, "Helvetica");
    pdf_append_page(pdf);

    resetX();
    resetY();
    lineWidth = pageDimensions[getPageSize()-1][0] - margins[getRightMargin()];

    loadOptions();

}

void exportCleanUp(BFXferRecPtr dataIn, BFResultOutPtr dataOut) {
    char *err = NULL;
    GSString255 filepath;
    FileInfoRecGS info = { 4, &filepath };
    pdf_save(pdf, outputFilePath);
    err = pdf_get_err(pdf, NULL);
    
    pdf_destroy(pdf);

    //update the filetype info from text 
    strcpy(filepath.text, outputFilePath);
    filepath.length = strlen(outputFilePath);
    GetFileInfoGS(&info);
    if (!toolerror()) {
        info.fileType = 0x54;
        info.auxType = 0x1080;
        SetFileInfoGS(&info);
    }

    SystemSANEShutDown();

}

static char* getFontFace(word id, word style) {
    word idx;
    switch (id) {
    case helvetica:
        idx = 0;
        break;
    case courier:
        idx = 1;
        break;
    case times:
        idx = 2;
        break;
    case symbol:
        idx = 3;
        break;
    case zapfDingbats:
        idx = 4;
        break;
    default:
        idx = getDefaultFont() - 1;
        break;
    }
    if ((style & (boldMask | italicMask)) == (boldMask | italicMask)) {
        return fontTable[idx][0];
    } else if ((style & boldMask) == boldMask) {
        return fontTable[idx][1];
    } else if ((style & italicMask) == italicMask) {
        return fontTable[idx][2];
    } else {
        return fontTable[idx][3];
    }
}


static const char *find_word_break(const char *string, const char *end) {
    /* Skip over the actual word */
    while (string && *string && !isspace(*string) && *string != 0xCA && (string < end)) {
        string++;
    }

    return string;
}

static bool HorizontalLineCheck(const char *cur, const char *end) {
    while (cur != end) {
        if (*cur != '_') {
            return false;
        }
        cur++;
    }
    if (*cur != '\r' && *cur != '\n') {
        return false;
    }
    return true;
}

static uint32_t getRBGColor(word id) {
    word idx = 0;
    while (colors[idx].colorName != 0) {
        if (colors[idx].id == id) {
            return colors[idx].rgb;
        }
        idx++;
    }
    return 0;
}

void exportData(BFXferRecPtr dataIn) {
    TextData *td = (TextData *) dataIn->dataRecordPtr;
    char *start = td->textStreamPtr;
    char *cur = start;
    char *last = start;
    char *end = start + td->textStreamLen;
    char *fontFace = getFontFace(td->bfFontID, td->fontStyle);
    char *p = NULL;
    const uint16_t *widths = find_font_widths(fontFace);
    float size = td->fontSize;
    float spacing;
    float t = 0;
    int e;

    if ((td->bfFontID == SIS_FONT_3) || (td->bfFontID == SIS_FONT_4)) {
        return;
    }

    if (lastFontFace != fontFace) {
        pdf_set_font(pdf, fontFace);
        lastFontFace = fontFace;
    }

    //adjust y if the next line starts with a fontsize greater than
    //the last line. Not a perfect solution if the line contains 
    //multiple font sizes.
    if (x == margins[getLeftMargin()] && size != lastMaxSize) {
        t = size;
        t = lastMaxSize;
        t = (size - lastMaxSize);
        t = getSpacing();
        y -= (size - lastMaxSize) * spacings[getSpacing()];
        lastMaxSize = 0;
    }

    while (start < end) {
        int output = 0, wrap = 0;
        float textWidth;
        cur = find_word_break(cur, end);

        if (x == margins[getLeftMargin()] && *start == '_' && HorizontalLineCheck(start, cur)) {
            maxSize = (size > maxSize) ? size : maxSize;
            spacing = maxSize * spacings[getSpacing()];

            cur++;
            start = cur;
            last = cur;
            pdf_add_line(pdf, NULL, x, y, lineWidth, y, 4, PDF_BLACK);
            y -= (spacing*2);
            resetX();
            maxSize = 0;
        } else {

            e = pdf_text_point_width(pdf, start, cur - start, size, widths,
                                     &textWidth);
            if (e < 0) {
                pdf_clear_err(pdf);
                start = last = cur;
                continue;
            }

            if (x + textWidth >= lineWidth) {
                if (last == start) { 
                    //single word that is too long
                    ptrdiff_t i;
                    /* Find the best character to chop it at */
                    for (i = cur - start - 1; i > 0; i--) {
                        e = pdf_text_point_width(pdf, start, i, size, widths,
                                                 &textWidth);
                        if (e < 0) {
                            return;
                        }
                        if (x + textWidth < lineWidth) {
                            break;
                        }
                    }
                    cur = start + i;
                } else {
                    cur = last;
                }
                output = 1;
                wrap = 1;
            } 

            if (cur == end || (*cur == '\n') || (*cur == '\r')) {
                output = 1;
            }

            if (output) {
                Handle h;
                uint32_t color = getRBGColor(td->foreColor);
                maxSize = (size > maxSize) ? size : maxSize;
                spacing = maxSize * spacings[getSpacing()];

                e = pdf_text_point_width(pdf, start, cur - start, size, widths,
                                             &textWidth);

                if (e < 0) {
                    pdf_clear_err(pdf);
                    start = last = cur;
                    continue;
                }
                if ((td->bfFontID == SIS_FONT_5) || (td->fontStyle & underlineMask)) {
                    pdf_add_line(pdf, NULL, x, y - 2, x + textWidth, y - 2, 1, PDF_BLACK);
                }
                h = NewHandle(cur - start + 1, userID, 0, NULL);
                p = *h;
                PtrToHand(start, h, cur - start);
                p[cur - start] = 0;
                //convert $CA used in webber links to a space
                while (*p) {
                    if (*p == 0xCA) {
                        *p = 0x20;
                    }
                    p++;
                }
                pdf_add_text(pdf, NULL, *h, size, x, y, color);
                DisposeHandle(h);
                if (wrap) {
                    resetX();
                    y -= spacing;
                    wrap = 0;
                    lastMaxSize = maxSize;
                    maxSize = 0;
                } else {
                    x += textWidth;
                }

                if (cur == end || (*cur == '\n') || (*cur == '\r')) {
                    if (cur - start == 1 && (*start == '\n' || (*start == '\r'))) {
                        resetX();
                        y -= spacing;
                        lastMaxSize = maxSize;
                        maxSize = 0;
                    } else if ((*cur == '\n') || (*cur == '\r')) {
                        resetX();
                        y -= spacing;
                        lastMaxSize = maxSize;
                        maxSize = 0;
                    }
                    cur++;
                }
                if (cur < end && *cur == ' ') {
                    //new line..skip leading space
                    cur++;
                }
                start = last = cur;
            } else {
                last = ++cur;
            }
            if (y <= margins[getBottomMargin()]) {
                pdf_append_page(pdf);
                resetY();
                resetX();
                maxSize = lastMaxSize = 0;
            }
        }
    } 
}

void trWrite(BFXferRecPtr dataIn, BFResultOutPtr dataOut) {
    TextData *td = (TextData *) dataIn->dataRecordPtr;

    if (dataIn->status == bfContinue) {
        switch (td->actionCode) {
        case bfGetSettings:
            memcpy(&exportFile, dataIn->filePathPtr, sizeof(GSString255));
            td->responseCode = bfTFReponseOK;
            break;
        case bfContinuing:
        case bfBodyText:
            exportData(dataIn);
            td->responseCode = bfTFReponseOK;
            break;
        default:
            td->responseCode = bfTFReponseSkip;
            break;
        }
    }
}

