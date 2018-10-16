FROM base/archlinux
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>
ARG njobs=2
ARG build_type=Release

# Installs the following versions (note that it might be out of date):
# cmake 3.12.3
# gmp 6.1.2
# boost 1.68.9
# python 3.7.0
# sqlite 3.25.2
# llvm 4.0.1
# clang 4.0.1
# gcc 8.2.1

# Upgrade
RUN pacman --noconfirm -Sy archlinux-keyring
RUN pacman --noconfirm -Syu
RUN pacman-db-upgrade

# Install most dependencies
RUN pacman -S --noconfirm base-devel cmake gmp boost boost-libs python sqlite libedit

# Use -j$njobs when building packages
RUN echo "MAKEFLAGS=\"-j$njobs\"" >> /etc/makepkg.conf

# Create user 'yay' with sudo access
RUN useradd -m yay
RUN echo "yay ALL=(root) NOPASSWD: /usr/bin/pacman" >> /etc/sudoers

# Install git, required to install yay
RUN pacman -S --noconfirm git

# Install yay
USER yay
WORKDIR /home/yay
RUN git clone https://aur.archlinux.org/yay.git
WORKDIR /home/yay/yay
RUN makepkg -si --noconfirm

# Install llvm 4
RUN yay -S --noconfirm --mflags="--nocheck" llvm40 clang40

# Add ikos source code
USER root
ADD . /root/ikos

# Build ikos
RUN rm -rf /root/ikos/build && mkdir /root/ikos/build
WORKDIR /root/ikos/build
ENV MAKEFLAGS "-j$njobs"
RUN cmake \
        -DCMAKE_INSTALL_PREFIX="/opt/ikos" \
        -DCMAKE_BUILD_TYPE="$build_type" \
        -DLLVM_CONFIG_EXECUTABLE="/usr/bin/llvm-config-4.0" \
        ..
RUN make
RUN make install

# Run the tests
RUN make check

# Add ikos to the path
ENV PATH "/opt/ikos/bin:$PATH"

# Done
WORKDIR /
