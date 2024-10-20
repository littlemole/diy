# This is a comment
FROM ubuntu:24.04
LABEL AUTHOR="me <little.mole@oha7.org>"

# std dependencies
RUN DEBIAN_FRONTEND=noninteractive apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get upgrade -y
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential g++ \
libgtest-dev cmake git pkg-config valgrind sudo joe wget \
clang libc++-dev libc++abi-dev

ARG CXX
ENV CXX=${CXX}

ARG BUILDCHAIN
ENV BUILDCHAIN=${BUILDCHAIN}

ARG WITH_TEST=On
ENV WITH_TEST=${WITH_TEST}

# compile gtest with given compiler
RUN cd /usr/src/gtest && \
  if [ "$BUILDCHAIN" = "make" ] ; then \
  if [ "$CXX" = "g++" ] ; then \
    cmake . -DCMAKE_CXX_FLAGS="-std=c++20"  ; \
  else \
  cmake -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_CXX_FLAGS="-std=c++20 -stdlib=libc++" . ; \
  fi && \
  make && \
  cp /usr/src/googletest/googletest/lib/libgtest.a /usr/lib/x86_64-linux-gnu/libgtest.a ; \
  fi


#RUN mkdir -p /opt/workspace/diy
ADD . /usr/local/src/diy
ADD docker/build.sh /usr/local/bin/build.sh

RUN echo -e $CXX
#CMD 

RUN /usr/local/bin/build.sh diy

#RUN cd /usr/local/src/diy && make clean &&  make -e && make -e test && make -e build && make -e install

