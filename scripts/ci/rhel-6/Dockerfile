FROM rhel6
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>

# Installs the following versions (note that it might be out of date):
# cmake 2.8.12.2
# gmp 4.3.1
# boost 1.61.0
# python 2.7.12
# sqlite 3.6.20
# llvm 3.7.1
# clang 3.7.1
# gcc 6.1.0

# Subscribe
ARG RHEL_USERNAME
ARG RHEL_PASSWORD
RUN subscription-manager register --username=$RHEL_USERNAME --password=$RHEL_PASSWORD --auto-attach

# Upgrade
RUN yum -y update

# Install dependencies
RUN yum -y install which patch tar bzip2 xz zlib-devel ncurses-devel sqlite-devel gmp-devel gcc gcc-c++ cmake

# Add ikos source code
ADD . /root/ikos

# Use the bootstrap script to install ikos
WORKDIR /root/ikos/scripts
RUN ./bootstrap -vf --prefix=/opt/ikos --builddir=/root/ikos-build

# Build the tests
WORKDIR /root/ikos-build/ikos-1.2
RUN source /opt/ikos/activate-full && make verifier-passes && make install

# Run the tests
RUN source /opt/ikos/activate-minimal && make test

# Done
WORKDIR /
