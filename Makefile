CXX = g++
DESTDIR=/
PREFIX=/usr/local

LIBNAME = diycpp
LIBINC = ./include

PWD = $(shell pwd)
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

BUILDCHAIN = make
CONTAINER = $(shell echo "$(LIBNAME)_$(CXX)_$(BUILDCHAIN)" | sed 's/++/pp/')
IMAGE = littlemole/$(CONTAINER)

WITH_TEST = On

all: test

test-build: ## make the test binaries
	cd t && make -e -f Makefile 
			
clean: ## cleans up build artefacts
	cd t && make -f Makefile clean
	-find -name "*~" -exec rm {} \;
	-rm -rf ./build
		
	
test: test-build ## runs unit tests
	bash -c 'BINS=$$(ls t/build/*.bin); for i in $$BINS; do $$i; if [ "$$?" != "0" ]; then echo "testrunner FAILED"; exit 1; fi; done; echo "testrunner OK";'


build: 
	mkdir -p ./build/include
	mkdir -p ./build/lib/pkgconfig
	cp -r include/* ./build/include/
	cp $(LIBNAME).pc ./build/lib/pkgconfig

install: 
	-rm -rf $(DESTDIR)/$(PREFIX)/include/$(LIBNAME)
	cp -r $(LIBINC)/* $(DESTDIR)/$(PREFIX)/include/
	mkdir -p $(DESTDIR)/$(PREFIX)/lib/pkgconfig/
	cp $(LIBNAME).pc $(DESTDIR)/$(PREFIX)/lib/pkgconfig/
	
remove: 
	-rm -rf $(DESTDIR)/$(PREFIX)/include/$(LIBNAME)
	-rm $(DESTDIR)/$(PREFIX)/lib/pkgconfig/$(LIBNAME).pc
	
image: 
	docker build -t $(IMAGE) . -fDockerfile  --build-arg CXX=$(CXX) --build-arg BUILDCHAIN=$(BUILDCHAIN) --build-arg WITH_TEST=$(WITH_TEST)


# docker stable testing environment

clean-image: 
	docker build -t $(IMAGE) . --no-cache -fDockerfile --build-arg CXX=$(CXX) --build-arg BUILDCHAIN=$(BUILDCHAIN) --build-arg WITH_TEST=$(WITH_TEST)

		
run: image-remove image 
	docker run --name diypp -d -e CXX=$(CXX) -v "$(PWD):/opt/workspace/diy"  $(IMAGE)
                                        
bash: image-remove image
	docker run --name diypp -ti -e CXX=$(CXX) -v "$(PWD):/opt/workspace/diy"  $(IMAGE) bash

stop: 
	-docker stop diypp
	
image-remove: stop 
	-docker rm diypp


package:
	rm -rf out

	cmake --preset "gcc-debug"
	cmake --build --preset "gcc-debug"
	DESTDIR=$(SCRIPT_DIR)_install cmake --build  --target install --preset="gcc-debug"

	cmake --preset "gcc-release"
	cmake --build --preset "gcc-release"
	DESTDIR=$(SCRIPT_DIR)_install cmake --build  --target install --preset="gcc-release"

	cpack --config release.cmake -G DEB

