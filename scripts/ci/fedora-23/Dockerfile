FROM fedora:23
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>

# Installs the following versions (note that it might be out of date):
# cmake 3.4.3
# gmp 6.0.0
# boost 1.58.0
# python 2.7.11
# sqlite 3.11.0
# llvm 3.7.0
# clang 3.7.0
# gcc 5.3.1

# Upgrade
RUN dnf -y update

# Install all dependencies
RUN dnf install -y make cmake python gmp-devel sqlite-devel zlib-devel libedit-devel \
        boost-devel gcc gcc-c++ llvm llvm-devel llvm-static clang

# Fix for LLVM cmake files
RUN sed -i 's#set(_LLVM_LIBRARY_DIR .*)#set(_LLVM_LIBRARY_DIR "${LLVM_INSTALL_PREFIX}/lib64/llvm")#' /usr/share/llvm/cmake/LLVMConfig.cmake

# Add ikos source code
ADD . /root/ikos

# Build ikos
RUN rm -rf /root/ikos/build && mkdir /root/ikos/build
WORKDIR /root/ikos/build
RUN cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/opt/ikos ..
RUN make all verifier-passes install

# Add ikos to the path
ENV IKOS_INSTALL "/opt/ikos"
ENV PATH "/opt/ikos/bin:$PATH"

# Run the tests
RUN make test

# Done
WORKDIR /
