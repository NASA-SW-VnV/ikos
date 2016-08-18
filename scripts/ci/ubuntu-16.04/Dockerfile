FROM ubuntu:16.04
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>

# Installs the following versions (note that it might be out of date):
# cmake 3.5.1
# gmp 6.1.0
# boost 1.58.0
# python 2.7.12
# sqlite 3.11.0
# llvm 3.7.1
# clang 3.7.1
# gcc 4.5.0

# Upgrade
RUN apt-get update
RUN apt-get upgrade -y

# Install all dependencies
RUN apt-get install -y cmake libgmp-dev libsqlite3-dev libz-dev libedit-dev \
        libboost-dev libboost-program-options-dev libboost-filesystem-dev \
        gcc g++ llvm-3.7 clang-3.7

# Update path to include llvm
ENV PATH "/usr/lib/llvm-3.7/bin:$PATH"

# Add ikos source code
ADD . /root/ikos

# Build ikos
RUN rm -rf /root/ikos/build && mkdir /root/ikos/build
WORKDIR /root/ikos/build
RUN cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/opt/ikos -DCMAKE_MODULE_PATH=/usr/share/llvm-3.7/cmake ..
RUN make all verifier-passes install

# Add ikos to the path
ENV IKOS_INSTALL "/opt/ikos"
ENV PATH "/opt/ikos/bin:$PATH"

# Run the tests
RUN make test

# Done
WORKDIR /
