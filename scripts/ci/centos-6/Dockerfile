FROM centos:6
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>

# Installs the following versions (note that it might be out of date):
# cmake 2.8.12.2
# gmp 4.3.1
# boost 1.61.0
# python 2.7.8
# sqlite 3.6.20
# llvm 3.7.1
# clang 3.7.1
# gcc 5.2.1

# Upgrade
RUN yum -y update

# Install dependencies using CentOS Sofware Collections Repository
RUN yum -y install patch bzip2 xz zlib-devel libedit-devel sqlite-devel gmp-devel cmake
RUN yum -y install centos-release-scl
RUN yum -y install devtoolset-4-gcc devtoolset-4-gcc-c++ python27

# Update env to use devtoolset-4 and python27
ENV PATH "/opt/rh/python27/root/usr/bin:/opt/rh/devtoolset-4/root/usr/bin:$PATH"
ENV LD_LIBRARY_PATH "/opt/rh/python27/root/usr/lib64:/opt/rh/devtoolset-4/root/usr/lib64:/opt/rh/devtoolset-4/root/usr/lib"

# Add ikos source code
ADD . /root/ikos

# Use the bootstrap script to install ikos
WORKDIR /root/ikos/scripts
RUN ./bootstrap -vf --prefix=/opt/ikos --builddir=/root/ikos-build

# Build the tests
WORKDIR /root/ikos-build/ikos-1.1.1
RUN source /opt/ikos/activate-full && make verifier-passes && make install

# Run the tests
RUN source /opt/ikos/activate-minimal && make test

# Done
WORKDIR /
