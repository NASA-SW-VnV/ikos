FROM centos:6
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>
ARG njobs=2
ARG build_type=Release

# Installs the following versions (note that it might be out of date):
# cmake 3.13.4
# gmp 4.3.1
# boost 1.69.0
# python 2.7.13
# sqlite 3.6.20
# llvm 8.0.0
# clang 8.0.0
# gcc 8.2.1

# Upgrade
RUN yum -y update

# Install dependencies using CentOS Sofware Collections Repository
RUN yum -y install patch bzip2 xz zlib-devel libedit-devel make gmp-devel sqlite-devel
RUN yum -y install centos-release-scl
RUN yum -y install devtoolset-8-gcc devtoolset-8-gcc-c++ python27

# Update env to use devtoolset-8 and python27
ENV PATH "/opt/rh/python27/root/usr/bin:/opt/rh/devtoolset-8/root/usr/bin:$PATH"
ENV LD_LIBRARY_PATH "/opt/rh/python27/root/usr/lib64:/opt/rh/devtoolset-8/root/usr/lib64:/opt/rh/devtoolset-8/root/usr/lib"

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
