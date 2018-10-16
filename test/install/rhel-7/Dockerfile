FROM samdoran/rhel7-ansible
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>
ARG njobs=2
ARG build_type=Release
ARG rhel_username
ARG rhel_password

# Installs the following versions (note that it might be out of date):
# cmake 3.12.3
# gmp 6.0.0
# boost 1.68.0
# python 2.7.5
# sqlite 3.7.17
# llvm 4.0.1
# clang 4.0.1
# gcc 8.2.0

# Subscribe
RUN subscription-manager register --username=$rhel_username --password=$rhel_password --auto-attach

# Upgrade
RUN yum -y update

# Install dependencies
RUN yum -y install which file patch tar bzip2 xz zlib-devel ncurses-devel sqlite-devel gmp-devel gcc gcc-c++ make python

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
