VERSION=5
MINOR_NUMBER=05

BIN=~/bin/
BIN=

#LIB=/usr/lib/cst/
LIB=~/bin/
LIB=

PNAME=cstlemma

#very big
PNAMESTATIC=$(BIN)$(PNAME)s
#big
PNAMEDYNAMIC=$(BIN)$(PNAME)
#small
PNAMEDYNAMICLIB=$(BIN)$(PNAME)dl

LINKERNAME=$(LIB)lib$(PNAME).so
SONAME=$(LINKERNAME).$(VERSION)
REALNAME=$(SONAME).$(MINOR_NUMBER)

SRCDIR     = .
HASHDIR    = ../../hashmap/src
LETTERFUNCDIR    = ../../letterfunc/src
SGMLDIR    = ../../parsesgml/src
INCLUDEDIR = -I$(SRCDIR) -I$(HASHDIR) -I$(LETTERFUNCDIR) -I$(SGMLDIR)

CC=g++ $(INCLUDEDIR) -O3 -Wall -pedantic -DNDEBUG 

# -fPIC or -fpic: enable 'position independent code' generation. Necessary for shared libs
# -fpic may generate smaller and faster code, but will have platform-dependent limitations
PIC=-fPIC
#PIC=

CCLINKSTATIC=g++ -static
CCLINKDYNAMIC=g++ 
CCCREATELIB=g++ -shared -Wl,-soname,$(SONAME)

#DEBUG=-g
DEBUG=

GCCLINK=-L/usr/local/lib -lstdc++

RM=rm -f

CSTLEMMASRC=cstlemma.cpp

CSTLEMMAOBJ=cstlemma.o

LEMMATISERSRC=\
	applyrules.cpp\
	argopt.cpp\
	basefrm.cpp\
	basefrmpntr.cpp\
	caseconv.cpp\
	dictionary.cpp\
	field.cpp\
	flattext.cpp\
	flex.cpp\
	freqfile.cpp\
	function.cpp\
	functiontree.cpp\
        $(HASHDIR)/hashmap.cpp \
	lemmatise.cpp\
	lemmatiser.cpp\
	lemmtags.cpp\
        $(LETTERFUNCDIR)/letter.cpp \
        $(LETTERFUNCDIR)/letterfunc.cpp \
	lext.cpp\
	makedict.cpp\
	makesuffixflex.cpp\
	option.cpp\
	outputclass.cpp\
	$(SGMLDIR)/parsesgml.cpp \
	readfreq.cpp\
	readlemm.cpp\
	tags.cpp\
	text.cpp\
 	$(LETTERFUNCDIR)/utf8func.cpp \
	word.cpp\
	wordReader.cpp\
	XMLtext.cpp

LEMMATISEROBJS=\
	applyrules.o\
	argopt.o\
	basefrm.o\
	basefrmpntr.o\
	caseconv.o\
	dictionary.o\
	field.o\
	flattext.o\
	flex.o\
	freqfile.o\
	function.o\
	functiontree.o\
	hashmap.o\
	lemmatise.o\
	lemmatiser.o\
	lemmtags.o\
	letter.o\
	letterfunc.o\
	lext.o\
	makedict.o\
	makesuffixflex.o\
	option.o\
	outputclass.o\
	parsesgml.o\
	readfreq.o\
	readlemm.o\
	tags.o\
	text.o\
	utf8func.o\
	word.o\
	wordReader.o\
	XMLtext.o

#                               #
#   Dependencies start here.    #
#                               #


$(PNAMEDYNAMIC): $(CSTLEMMAOBJ) $(LEMMATISEROBJS)
	$(CCLINKDYNAMIC) $(CSTLEMMAOBJ) $(LEMMATISEROBJS) -o $@ $(GCCLINK)

$(PNAMESTATIC): $(CSTLEMMAOBJ) $(LEMMATISEROBJS)
	$(CCLINKSTATIC) $(CSTLEMMAOBJ) $(LEMMATISEROBJS) -o $@ $(GCCLINK)

$(REALNAME) : $(LEMMATISEROBJS)
	$(CCCREATELIB) -o $@ $(LEMMATISEROBJS)
	ln -sf $(REALNAME) $(SONAME)
	ln -sf $(SONAME) $(LINKERNAME)

$(PNAMEDYNAMICLIB): $(REALNAME) $(CSTLEMMAOBJ)
	$(CCLINKDYNAMIC) $(CSTLEMMAOBJ) $(REALNAME) -o $@ $(GCCLINK)


all: $(PNAMESTATIC) $(PNAMEDYNAMIC) $(REALNAME) $(PNAMEDYNAMICLIB)

.cpp.o:
	$(CC) $(PIC) $(DEBUG) $(GCCINC) -c $(CSTLEMMASRC) $(LEMMATISERSRC)

clean:
	$(RM) *.o
	$(RM) $(REALNAME)
	$(RM) $(SONAME)
	$(RM) $(LINKERNAME)

