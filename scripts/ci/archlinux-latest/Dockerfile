FROM base/archlinux
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>

# Installs the following versions (note that it might be out of date):
# cmake 3.6.0
# gmp 6.1.1
# boost 1.60.0
# python 3.5.2
# sqlite 3.13.0
# llvm 3.8.0
# clang 3.8.0
# gcc 6.1.1

# Upgrade
RUN pacman --noconfirm -Sy archlinux-keyring
RUN pacman --noconfirm -Syu
RUN pacman-db-upgrade
RUN pacman --noconfirm -S ca-certificates-mozilla
RUN pacman --noconfirm -Syu

# Install all dependencies
RUN pacman --noconfirm -S base-devel cmake gmp boost boost-libs python sqlite \
        llvm llvm-libs clang

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
