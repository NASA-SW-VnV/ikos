FROM samdoran/rhel6-ansible
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>
ARG njobs=2
ARG build_type=Release
ARG rhel_username
ARG rhel_password

# Installs the following versions (note that it might be out of date):
# cmake 3.15.2
# gmp 6.1.2
# mpfr 4.0.2
# ppl 1.2
# apron 0.9.10
# boost 1.70.0
# python 3.4.10
# sqlite 3.6.20
# tbb 4001
# llvm 9.0.0
# clang 9.0.0
# gcc 9.2.0

# Subscribe
RUN subscription-manager register --username=$rhel_username --password=$rhel_password --auto-attach

# Upgrade
RUN yum -y update

# Install dependencies
RUN yum -y install which file patch tar bzip2 xz zlib-devel ncurses-devel gcc gcc-c++ make m4 python34 python34-pygments sqlite-devel tbb-devel

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
