FROM ubuntu:14.04
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>

# Installs the following versions (note that it might be out of date):
# cmake 2.8.12.2
# gmp 5.1.3
# boost 1.55.0
# python 2.7.6
# sqlite 3.8.2
# llvm 3.7.1
# clang 3.7.1
# gcc 4.9.3

# Upgrade
RUN apt-get update
RUN apt-get upgrade -y

# Install add-apt-repository
RUN apt-get install -y software-properties-common

# Add ppa for gcc-4.9 and g++-4.9
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test

# Add ppa for llvm 3.7
RUN add-apt-repository -y "deb http://apt.llvm.org/trusty/ llvm-toolchain-trusty-3.7 main"

# Refresh cache
RUN apt-get update

# Install all dependencies
RUN apt-get install -y cmake libgmp-dev libsqlite3-dev libz-dev libedit-dev \
        libboost1.55-dev libboost-program-options1.55-dev libboost-filesystem1.55-dev \
        gcc-4.9 g++-4.9
RUN apt-get install -y --force-yes llvm-3.7 clang-3.7

# Update path to include llvm
ENV PATH "/usr/lib/llvm-3.7/bin:$PATH"

# Add ikos source code
ADD . /root/ikos

# Build ikos
RUN rm -rf /root/ikos/build && mkdir /root/ikos/build
WORKDIR /root/ikos/build
RUN CC=gcc-4.9 CXX=g++-4.9 cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/opt/ikos -DCMAKE_MODULE_PATH=/usr/share/llvm-3.7/cmake ..
RUN make all verifier-passes install

# Add ikos to the path
ENV IKOS_INSTALL "/opt/ikos"
ENV PATH "/opt/ikos/bin:$PATH"

# Run the tests
RUN make test

# Done
WORKDIR /
