HEADERS = \
	  src/defs.h \
	  src/widget/defs.h \
	  \
	  src/buf.h \
	  src/output.h \
	  src/pack.h \
	  src/pt.h \
	  src/scr.h \
	  \
	  src/widget/edit.h \
	  src/widget/handler.h \
	  src/widget/index.h \
	  src/widget/pane.h \
	  src/widget/resp.h \
	  src/widget/select.h \
	  src/widget/ui.h \
	  src/widget/widget.h \

all: scr.h

scr.h: $(HEADERS) user.mk
	printf '#ifndef LIBSCR_H\n#define LIBSCR_H\n' > scr.h
	for file in $(HEADERS) ; do sed -e '1,/%scr.h%/d' -e '/%~scr.h%/,$$d' $$file >> scr.h ; done
	printf '#endif\n' >> scr.h

install: scr_h_install

scr_h_install: scr.h
	install --mode 0644 -D scr.h "$(PREFIX)/include/scr.h"

clean: scr_h_clean

scr_h_clean:
	rm -f scr.h

.PHONY: scr_h_clean
