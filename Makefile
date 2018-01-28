CXX = g++
DESTDIR=/
PREFIX=/usr/local

LIBNAME = diycpp
LIBINC = ./include

all: test

test-build: ## make the test binaries
	cd t && make -e -f Makefile 
			
clean: ## cleans up build artefacts
	cd t && make -f Makefile clean
	-find -name "*~" -exec rm {} \;
	-rm -rf ./build
		
	
test: test-build ## runs unit tests
	./t/build/test.bin

build: 
	mkdir -p ./build/include
	mkdir -p ./build/lib/pkgconfig
	cp -r include/* ./build/include/
	cp $(LIBNAME).pc ./build/lib/pkgconfig

install: 
	-rm -rf $(DESTDIR)/$(PREFIX)/include/$(LIBNAME)
	cp -r $(LIBINC) $(DESTDIR)/$(PREFIX)/include/$(LIBNAME)
	cp $(LIBNAME).pc $(DESTDIR)/$(PREFIX)/lib/pkgconfig/
	
remove: 
	-rm -rf $(DESTDIR)/$(PREFIX)/include/$(LIBNAME)
	-rm $(DESTDIR)/$(PREFIX)/lib/pkgconfig/$(LIBNAME).pc
	
