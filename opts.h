#ifndef __OPTS_H__
#define __OPTS_H__

typedef struct {
    word version;
    word defFont;
    word pageSize;
    word spacing;
    word topMargin;
    word leftMargin;
    word rightMargin;
    word bottomMargin;
} opts;

#define PREF_VERSION 1

void loadOptions(void);
void saveOptions(void);
word getDefaultFont(void);
void setDefaultFont(word id);
word getPageSize(void);
void setPageSize(word size);
word getSpacing(void);
void setSpacing(word size);
word getTopMargin(void);
void setTopMargin(word margin);
word getLeftMargin(void);
void setLeftMargin(word margin);
word getRightMargin(void);
void setRightMargin(word margin);
word getBottomMargin(void);
void setBottomMargin(word margin);


#endif
