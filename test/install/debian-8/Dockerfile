FROM debian:8
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>
ARG njobs=2
ARG build_type=Release

# Installs the following versions (note that it might be out of date):
# cmake 3.14.1
# gmp 6.0.0
# boost 1.55.0
# python 2.7.9
# sqlite 3.8.7.1
# llvm 8.0.0
# clang 8.0.0
# gcc 4.9.2

# Upgrade
RUN apt-get update
RUN apt-get upgrade -y

# Add ppa for llvm 8.0
RUN echo "deb http://apt.llvm.org/jessie/ llvm-toolchain-jessie-8 main" >> /etc/apt/sources.list

# Add llvm repository key
RUN apt-get install -y wget gnupg
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

# Install cmake
WORKDIR /tmp
RUN wget https://github.com/Kitware/CMake/releases/download/v3.14.1/cmake-3.14.1-Linux-x86_64.tar.gz
RUN tar xf cmake-3.14.1-Linux-x86_64.tar.gz
ENV PATH "/tmp/cmake-3.14.1-Linux-x86_64/bin:$PATH"

# Refresh cache
RUN apt-get update

# Install all dependencies
RUN apt-get install -y gcc g++ make libgmp-dev libboost-dev \
        libboost-filesystem-dev libboost-test-dev python python-pygments \
        libsqlite3-dev libz-dev libedit-dev \
        llvm-8 llvm-8-dev llvm-8-tools clang-8

# Add ikos source code
ADD . /root/ikos

# Build ikos
RUN rm -rf /root/ikos/build && mkdir /root/ikos/build
WORKDIR /root/ikos/build
ENV MAKEFLAGS "-j$njobs"
RUN cmake \
        -DCMAKE_INSTALL_PREFIX="/opt/ikos" \
        -DCMAKE_BUILD_TYPE="$build_type" \
        -DLLVM_CONFIG_EXECUTABLE="/usr/lib/llvm-8/bin/llvm-config" \
        -DLLVM_TEMPORARILY_ALLOW_OLD_TOOLCHAIN=ON \
        ..
RUN make
RUN make install

# Run the tests
RUN make check

# Add ikos to the path
ENV PATH "/opt/ikos/bin:$PATH"

# Done
WORKDIR /
