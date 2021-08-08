# This make file is used to build the babelfish ORCA Shell Utilily using 
# the Golden Gate compatibility layer by Kelvin Sherlock.
#
# Compiling using the ORCA Shell use the following commands:
# compile main.c keep=main
# compile getopt.c keep=getopt
# compile babelStuff.c keep=babelStuff
# set KeepType EXE
# set AuxType $0x100
# link main getopt babelstuff keep=babelStuff

CC=occ
_BABEL_OBJ= init.a dispatch.a export.a pdfgen.a opts.a strcasecmp.a
_EXPORT_OPT_OBJ = exportOpt.a opts.a

BABEL_OBJ=$(patsubst %,$(ODIR)/%,$(_BABEL_OBJ))
EXPORT_OPT_OBJ=$(patsubst %,$(ODIR)/%,$(_EXPORT_OPT_OBJ))

ODIR=o
DEPS= 

all: PDF.Writer $(ODIR)/._PDF.Writer.r


$(ODIR)/init.a: init.c

$(ODIR)/dispatch.a: dispatch.c

$(ODIR)/export.a: export.c export.h

$(ODIR)/exportOpts.a: exportOpts.c opts.h

$(ODIR)/opts.a: opts.c opts.h

$(ODIR)/strcasecmp.a: strcasecmp.c strcasecmp.h

PDF.Writer: $(ODIR)/babel
	@touch $@
	@iix chtyp -t 0x00BE -a 0x4003 $@

$(ODIR)/babel : $(BABEL_OBJ)
	iix link -P $(patsubst $(ODIR)/%.a,$(ODIR)/%,$(BABEL_OBJ)) keep=$(basename $@)

$(ODIR)/exportOpt : $(EXPORT_OPT_OBJ)
	iix link -P $(patsubst $(ODIR)/%.a,$(ODIR)/%,$(EXPORT_OPT_OBJ)) keep=$(basename $@)

$(ODIR)/%.a: %.c $(DEPS)
	@mkdir -p o
	$(CC) -O -1 --lint -1 -c -o $@ $< 

$(ODIR)/._PDF.Writer.r: pdfrez.rez pdfrez.equ $(ODIR)/babel $(ODIR)/exportOpt
	@mkdir -p o
	$(CC) -o $(ODIR)/PDF.Writer.r pdfrez.rez
	cp $@ ._PDF.Writer

clean:
	@rm -f $(ODIR)/*.a $(ODIR)/*.root $(ODIR)/pdfgen PDF.Writer $(ODIR)/exportOpt
	@rm -f $(ODIR)/babel $(ODIR)/filter $(ODIR)/PDF.Writer.r $(ODIR)/._PDF.Writer.r
	@rm -f ._PDF.Writer
