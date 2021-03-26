CC:=c99
CFLAGS:=
LIBS:="-pthread"

M4:=m4
M4FLAGS:=

TAR:=tar
TARFLAGS:=czf
ZIP:=gzip
ZIPFLAGS:=-c
ZIPEXTRA:=.gz

srcdir:=src
BIN:=mtcat
DESTDIR:=/usr/bin

docdir:=doc
DOC:=$(BIN).1
MANDIR:=/usr/share/man/man1

INSTALL:=install.sh

MAJOR:=1
MINOR:=0
PATCH:=0
EXTRA:=rc
VERSION:=$(MAJOR).$(MINOR).$(PATCH)$(EXTRA)


.PHONEY: all clean install unistall dist doc

all: $(BIN)

doc: $(DOC)$(ZIPEXTRA)

dist: $(BIN) $(DOC)$(ZIPEXTRA) $(INSTALL)
	$(TAR) $(TARFLAGS) $(BIN).$(VERSION).tar$(ZIPEXTRA) $(BIN) $(DOC)$(ZIPEXTRA) $(INSTALL)

clean:
	rm -f $(BIN) $(DOC)$(ZIPEXTRA) $(BIN).$(VERSION).tar$(ZIPEXTRA) $(INSTALL)

install: $(BIN) $(DOC)$(ZIPEXTRA)
	cp $(BIN) $(DESTDIR)/$(BIN)
	cp $(DOC)$(ZIPEXTRA) $(MANDIR)/$(DOC)$(ZIPEXTRA)

unistall:
	rm $(DESTDIR)/$(BIN) $(MANDIR)/$(DOC)


$(BIN): $(srcdir)/main.c
	$(CC) $^ $(LIBS) -o $@

$(INSTALL): install.m4
	$(M4) $(M4FLAGS) -DDOC=$(DOC) -DMANDIR=$(MANDIR) -DBIN=$(BIN) -DDESTDIR=$(DESTDIR) $^ > $@
	chmod 755 $@

$(DOC)$(ZIPEXTRA): $(docdir)/$(DOC)
	$(ZIP) $(ZIPFLAGS) $(docdir)/$(DOC) > $(DOC)$(ZIPEXTRA)