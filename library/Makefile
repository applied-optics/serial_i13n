include ../config.mk

.PHONY : all install clean dist distclean

all : libserial_i13n.so.0

libserial_i13n.so.0 : serial_i13n.o
	$(CC) -shared -Wl,-soname,libserial_i13n.so.0 $^ -o $@

serial_i13n.o: serial_i13n.c serial_i13n.h
	$(CC) -fPIC $(CFLAGS) -c $< -o $@

TAGS: $(wildcard *.c) $(wildcard *.h) $(wildcard *.c)
	etags $^

clean:
	rm -f *.o libserial_i13n.so.0 TAGS

install: all
	$(INSTALL) -d $(DESTDIR)$(prefix)/lib${LIB_SUFFIX}/
	$(INSTALL) libserial_i13n.so.0 $(DESTDIR)$(prefix)/lib${LIB_SUFFIX}/
	ln -sf libserial_i13n.so.0 $(DESTDIR)$(prefix)/lib${LIB_SUFFIX}/libserial_i13n.so
	$(INSTALL) -d $(DESTDIR)$(prefix)/include/i13n/
	$(INSTALL) serial_i13n.h $(DESTDIR)$(prefix)/include/

