FROM archlinux/base
MAINTAINER Maxime Arthaud <maxime.arthaud@nasa.gov>
ARG njobs=2
ARG build_type=Release

# Installs the following versions (note that it might be out of date):
# cmake 3.14.1
# gmp 6.1.2
# mpfr 4.0.2
# ppl 1.2
# apron 0.9.10
# boost 1.69.0
# python 3.7.3
# sqlite 3.27.2
# llvm 8.0.0
# clang 8.0.0
# gcc 8.2.1

# Upgrade
RUN pacman --noconfirm -Sy archlinux-keyring
RUN pacman --noconfirm -Syu
RUN pacman-db-upgrade

# Install most dependencies
RUN pacman -S --noconfirm base-devel git cmake gmp boost boost-libs python python-pygments sqlite llvm llvm-libs clang

# Use -j$njobs when building packages
RUN echo "MAKEFLAGS=\"-j$njobs\"" >> /etc/makepkg.conf

# Create user 'makepkg' with sudo access
RUN useradd -m makepkg
RUN echo "makepkg ALL=(root) NOPASSWD: /usr/bin/pacman" >> /etc/sudoers

# Install apron using AUR
USER makepkg
WORKDIR /home/makepkg
RUN git clone https://aur.archlinux.org/apron.git
WORKDIR /home/makepkg/apron
RUN makepkg -si --noconfirm

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
