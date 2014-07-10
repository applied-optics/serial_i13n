VERSION=1.0

include config.mk

DIRS=library

.PHONY : all clean install

all :
	for d in ${DIRS}; do $(MAKE) -C $${d}; done

clean :
	for d in ${DIRS}; do $(MAKE) -C $${d} clean; done

install : all
	for d in ${DIRS}; do $(MAKE) -C $${d} install; done

dist : distclean
	mkdir serial_i13n-$(VERSION)
	cp -pr library serial_i13n-$(VERSION)/
	cp -p Makefile CHANGELOG.txt README.txt GNU_General_Public_License.txt serial_i13n-$(VERSION)/
	tar -zcf serial_i13n-$(VERSION).tar.gz serial_i13n-$(VERSION)

distclean : 
	rm -rf serial_i13n-$(VERSION)
	rm -f serial_i13n-$(VERSION).tar.gz
