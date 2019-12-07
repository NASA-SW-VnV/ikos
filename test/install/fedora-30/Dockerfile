FROM fedora:30
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>
ARG njobs=2
ARG build_type=Release

# Installs the following versions (note that it might be out of date):
# cmake 3.14.5
# gmp 6.1.2
# mpfr 3.1.6
# ppl 1.2
# apron 0.9.10
# boost 1.69.0
# python 2.7.17
# sqlite 3.26.0
# tbb 11008
# llvm 9.0.0
# clang 9.0.0
# gcc 9.2.1

# Upgrade
RUN dnf -y update

# Install all dependencies
RUN dnf install -y which findutils patch bzip2 xz zlib-devel libedit-devel \
        gcc gcc-c++ make cmake m4 gmp-devel mpfr-devel ppl-devel boost-devel \
        python python-pygments sqlite-devel tbb-devel

# Add ikos source code
ADD . /root/ikos

# Use the bootstrap script to install ikos and run the tests
WORKDIR /root/ikos/script
RUN ./bootstrap -vvf \
        --prefix="/opt/ikos" \
        --builddir="/root/ikos-build" \
        --build-type="$build_type" \
        --jobs="$njobs"

# Done
WORKDIR /
