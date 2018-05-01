# This is a comment
FROM ubuntu:18.04
MAINTAINER me <little.mole@oha7.org>

# std dependencies
RUN DEBIAN_FRONTEND=noninteractive apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get upgrade -y
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential g++ \
libgtest-dev cmake git pkg-config valgrind sudo joe wget \
clang libc++-dev libc++abi-dev

ARG CXX
ENV CXX=${CXX}

# compile gtest with given compiler
RUN cd /usr/src/gtest && \
  if [ "$CXX" = "g++" ] ; then \
    cmake .; \
  else \
  cmake -DCMAKE_CXX_COMPILER=$CXX -DCMAKE_CXX_FLAGS="-std=c++14 -stdlib=libc++" . ; \
  fi && \
  make && \
  ln -s /usr/src/gtest/libgtest.a /usr/lib/libgtest.a

RUN mkdir -p /opt/workspace/diy

RUN echo -e $CXX
CMD ["/bin/bash", "-c", "cd /opt/workspace/diy; make clean; make -e; make -e test; make -e build; make -e install;"]
