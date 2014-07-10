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
	mkdir serial-$(VERSION)
	cp -pr library utils serial-$(VERSION)/
	cp -p Makefile CMakeLists.txt CHANGELOG.txt README.txt GNU_General_Public_License.txt serial-$(VERSION)/
	tar -zcf serial-$(VERSION).tar.gz serial-$(VERSION)

distclean : 
	rm -rf serial-$(VERSION)
	rm -f serial-$(VERSION).tar.gz
