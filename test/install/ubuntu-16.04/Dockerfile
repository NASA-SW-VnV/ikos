FROM ubuntu:16.04
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>
ARG njobs=2
ARG build_type=Release

# Installs the following versions (note that it might be out of date):
# cmake 3.5.1
# gmp 6.1.0
# boost 1.58.0
# python 2.7.12
# sqlite 3.11.0
# llvm 4.0.0
# clang 4.0.0
# gcc 5.4.0

# Upgrade
RUN apt-get update
RUN apt-get upgrade -y

# Install all dependencies
RUN apt-get install -y cmake libgmp-dev libsqlite3-dev libz-dev libedit-dev \
        libboost-dev libboost-filesystem-dev libboost-test-dev \
        gcc g++ llvm-4.0 llvm-4.0-dev clang-4.0

# Update path to include llvm
ENV PATH "/usr/lib/llvm-4.0/bin:$PATH"

# Add ikos source code
ADD . /root/ikos

# Build ikos
RUN rm -rf /root/ikos/build && mkdir /root/ikos/build
WORKDIR /root/ikos/build
ENV MAKEFLAGS "-j$njobs"
RUN cmake \
        -DCMAKE_INSTALL_PREFIX="/opt/ikos" \
        -DCMAKE_BUILD_TYPE="$build_type" \
        ..
RUN make
RUN make install

# Run the tests
RUN make check

# Add ikos to the path
ENV PATH "/opt/ikos/bin:$PATH"

# Done
WORKDIR /
