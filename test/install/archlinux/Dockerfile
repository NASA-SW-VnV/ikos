FROM base/archlinux
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>
ARG njobs=2
ARG build_type=Release

# Installs the following versions (note that it might be out of date):
# cmake 3.12.4
# gmp 6.1.2
# mpfr 4.0.1
# ppl 1.2
# apron 0.9.11
# boost 1.68.0
# python 3.7.1
# sqlite 3.25.3
# llvm 7.0.0
# clang 7.0.0
# gcc 8.2.1

# Upgrade
RUN pacman --noconfirm -Sy archlinux-keyring
RUN pacman --noconfirm -Syu
RUN pacman-db-upgrade

# Install most dependencies
RUN pacman -S --noconfirm base-devel cmake gmp boost boost-libs python python-pygments sqlite llvm llvm-libs clang

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

# Install apron
RUN yay -S --noconfirm apron-ppl-svn

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
        ..
RUN make
RUN make install

# Run the tests
RUN make check

# Add ikos to the path
ENV PATH "/opt/ikos/bin:$PATH"

# Done
WORKDIR /
