CC:=c99
CFLAGS:=
LIBS:="-pthread"

M4:=m4
M4FLAGS:=

TAR:=tar
TARFLAGS:=cvzf

srcdir:=src
BIN:=mtcat
DESTDIR:=/usr/bin

docdir:=doc
DOC:=$(docdir)/$(BIN).1
MANDIR:=/usr/share/man

MAJOR:=1
MINOR:=0
PATCH:=0
EXTRA:=rc
VERSION:=.$(MAJOR).$(MINOR).$(PATCH)$(EXTRA)


.PHONEY: all clean install unistall dist doc

all: $(BIN)

$(BIN): $(srcdir)/main.c $(srcdir)/include/config.h
	$(CC) $^ $(LIBS) -o $@

$(srcdir)/include/config.h: $(srcdir)/include/config.h.m4
	$(M4) $(M4FLAGS) -DM4PROG=$(BIN) $^ > $@

doc: $(DOC)

dist: $(BIN)
	$(TAR) $(TARFLAGS) $(BIN).$(VERSION).tar.gz $(BIN) $(DOC)

clean:
	rm -rf $(BIN) $(srcdir)/include/config.h

install:
	mv $(BIN) $(DESTDIR)/$(BIN)

unistall:
	rm -f $(DESTDIR)/$(BIN)