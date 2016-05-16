#!/bin/bash

################################################################################
#
# Installation script for Red Hat (see notes below).
#
# Author: Maxime Arthaud
#
# Contact: ikos@lists.nasa.gov
#
# Notices:
#
# Copyright (c) 2011-2016 United States Government as represented by the
# Administrator of the National Aeronautics and Space Administration.
# All Rights Reserved.
#
# Disclaimers:
#
# No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
# ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
# TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
# ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
# OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
# ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
# THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
# ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
# RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
# RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
# DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
# IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
#
# Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
# THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
# AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
# IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
# USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
# RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
# HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
# AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
# RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
# UNILATERAL TERMINATION OF THIS AGREEMENT.
#
################################################################################
#
# The following script will install IKOS and all needed dependencies by itself.
# It has been tested on RHEL server 7.1, arch x86_64. We believe it should work
# on any RHEL 6.x and 7.x
#
# IKOS requires certain versions of CMake, GCC, LLVM and Boost that are newer
# than the ones available on the Red Hat Yum package manager. Hence the following
# describes the steps to bootstrap these dependencies on Red Hat.
#
# First, you will need a few yum packages. You can install them using:
#
# ```
# sudo yum install subversion bzip2 sqlite-devel gcc gcc-c++
# ```
#
# Now, simply run:
#
# ```
# ./bootstrap-redhat.sh /path/to/ikos-install /path/to/ikos-build /path/to/ikos-source
# ```
# 
# It might take a lot of time (a few hours) to complete.
#
# **Warning**: You will need at least 10G of disk space. 
#
# You can also install everything manually, by executing the following instructions.
# We recommend you to try to launch this script first.
# 
# Manual installation instructions
# ================================
#
# Install CMake 3.5.2
# -------------------
# 
# You will need a more recent version of CMake. In the following steps, we use
# /path/to/cmake-install as the installation directory for CMake. Replace it with
# the location where you want to put cmake (~/cmake-install for instance).
# 
# Download and extract CMake 3.5.2:
# 
# ```
# $ curl -O https://cmake.org/files/v3.5/cmake-3.5.2-Linux-x86_64.tar.gz
# $ tar xf cmake-3.5.2-Linux-x86_64.tar.gz -C /path/to/cmake-install
# ```
# 
# Now, add CMake to your PATH (consider adding this in your *.bashrc*):
# 
# ```
# PATH="/path/to/cmake-install/cmake-3.5.2-Linux-x86_64/bin:$PATH"
# ```
# 
# CMake is now ready! ```cmake --version``` should give you "cmake version 3.5.2".
# 
# Install GCC 6.1.0
# -----------------
# 
# We will build GCC under /path/to/gcc-build and install it under
# /path/to/gcc-install. Again, replace it with your own directories (for instance
# ~/gcc-build and ~/gcc-install).
# 
# Download and extract GCC source code:
# 
# ```
# $ curl -O ftp://gcc.gnu.org/pub/gcc/releases/gcc-6.1.0/gcc-6.1.0.tar.gz
# $ tar xf gcc-6.1.0.tar.gz -C /path/to/gcc-build
# $ cd /path/to/gcc-build/gcc-6.1.0/
# $ curl -O ftp://gcc.gnu.org/pub/gcc/infrastructure/mpc-1.0.3.tar.gz
# $ tar xf mpc-1.0.3.tar.gz
# $ mv mpc-1.0.3 mpc
# $ curl -O ftp://gcc.gnu.org/pub/gcc/infrastructure/isl-0.16.1.tar.bz2
# $ tar xf isl-0.16.1.tar
# $ mv isl-0.16.1 isl
# ```
# 
# Build and install GCC:
# 
# ```
# $ mkdir build
# $ cd build
# $ ../configure --prefix=/path/to/gcc-install --enable-languages=c,c++ --enable-threads=posix --enable-threads=posix --enable-__cxa_atexit --with-isl --disable-multilib
# $ make -j$(nproc)
# $ make install
# ```
# 
# Now, add GCC to your PATH and define CC, CXX and LDFLAGS (consider adding this
# in your *.bashrc*):
# 
# ```
# $ PATH="/path/to/gcc-install/bin:$PATH"
# $ export CC="/path/to/gcc-install/bin/gcc"
# $ export CXX="/path/to/gcc-install/bin/g++"
# $ export LDFLAGS="-L/path/to/gcc-install/lib64 -Wl,-rpath,/path/to/gcc-install/lib64"
# ```
# 
# GCC is now successfully installed!
# You might want to check that ```gcc --version``` gives you 6.1.0
# 
# Install LLVM and Clang 3.7.1
# ----------------------------
# 
# We will build LLVM and clang under /path/to/llvm-build and install them in
# /path/to/llvm-install. Again, replace it with your own directories.
# 
# Firs, download LLVM and clang source codes:
# 
# ```
# $ cd /path/to/llvm-build
# $ svn co http://llvm.org/svn/llvm-project/llvm/tags/RELEASE_371/final llvm-release-371
# $ cd llvm-release-371
# $ svn co http://llvm.org/svn/llvm-project/cfe/tags/RELEASE_371/final tools/clang
# $ svn co http://llvm.org/svn/llvm-project/compiler-rt/tags/RELEASE_371/final projects/compiler-rt
# ```
# 
# Build and install LLVM and clang:
# 
# ```
# $ mkdir build
# $ cd build
# $ cmake -DCMAKE_INSTALL_PREFIX=/path/to/llvm-install -DCMAKE_BUILD_TYPE=Release -DLLVM_TARGETS_TO_BUILD=X86 -DLLVM_ENABLE_RTTI=ON -DLLVM_ENABLE_EH=ON -DLLVM_ENABLE_CXX1Y=ON ..
# $ make -j$(nproc)
# $ make install
# ```
# 
# Now, add LLVM to your PATH (consider adding this in your *.bashrc*):
# 
# ```
# $ PATH="/path/to/llvm-install/bin:$PATH"
# ```
# 
# LLVM is now installed. You can check that ```clang --version``` returns 3.7.1
# 
# Install Boost 1.60.0
# --------------------
# 
# We will now build boost under /path/to/boost-build and install it under
# /path/to/boost-install. Again, replace it with your own directories.
# 
# Download and extract boost source code:
# 
# ```
# $ curl -OL https://sourceforge.net/projects/boost/files/boost/1.60.0/boost_1_60_0.tar.gz
# $ tar xf boost_1_60_0.tar.gz -C /path/to/boost-build
# $ cd /path/to/boost-build/boost_1_60_0
# ```
# 
# Build and install boost:
# 
# **Warning**: when you specify /path/to/boost-install to bootstrap.sh, do not use ~
# 
# ```
# $ ./bootstrap.sh --prefix=/path/to/boost-install --with-libraries=program_options
# $ ./b2 install
# ```
# 
# Now, set your BOOST_ROOT and LDFLAGS (consider adding this is your *.bashrc*):
# 
# ```
# $ export BOOST_ROOT=/path/to/boost-install
# $ export LDFLAGS="$LDFLAGS -L/path/to/boost-install/lib -Wl,-rpath,/path/to/boost-install/lib"
# ```
# 
# You are now ready to build IKOS.
#
################################################################################

progname=$(basename "$0")

if [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    echo "usage: $progname [-h] INSTALL-DIR BUILD-DIR IKOS-SRC"
    echo ""
    echo "Build and install IKOS on any UNIX environment, and without root permissions."
    echo ""
    echo "positional arguments:"
    echo "  INSTALL-DIR  The installation directory for IKOS and all dependencies"
    echo "  BUILD-DIR    The build directory for IKOS and all dependencies"
    echo "  IKOS-SRC     IKOS source directory"
    echo ""
    echo "optional arguments:"
    echo "  -h, --help  show this help message and exit"
    exit 0
fi

if [ "$#" -ne 3 ]; then
    echo "usage: $progname [-h] INSTALL-DIR BUILD-DIR IKOS-SRC"
    echo "$progname: error: too few arguments."
    exit 1
fi

if [ $UID -eq 0 ]; then
    echo "usage: $progname [-h] INSTALL-DIR BUILD-DIR IKOS-SRC"
    echo "$progname: error: do not run this script as root."
    exit 1
fi

#
# Generic functions
#

abspath() {
    # readlink -f as a different behavior on MAC OS
    # Try to use python os.path.abspath() first
    if command -v python2 >/dev/null 2>&1; then
        echo "$1" | python2 -c "import os;print os.path.abspath(raw_input())"
    elif command -v python3 >/dev/null 2>&1; then
        echo "$1" | python3 -c "import os;print(os.path.abspath(input()))"
    elif command -v readlink >/dev/null 2>&1; then
        readlink -f "$1"
    else
        echo "$progname: error: no available tool to implement abspath()."
        exit 1
    fi
}

download() {
    filename=$(basename "$1")
    if [ -f "$filename" ]; then
        echo "[-] Using already downloaded $filename"
        return
    fi

    # Try to use wget or curl
    if command -v wget >/dev/null 2>&1; then
        wget "$1"
    elif command -v curl >/dev/null 2>&1; then
        curl -OL "$1"
    else
        echo "$progname: error: no available tool to download $1."
        exit 1
    fi
}

version_compare() {
    if [[ $1 == $2 ]]; then
        return 0
    fi

    local IFS=.
    local i ver1=($1) ver2=($2)

    # fill empty fields in ver1 with zeros
    for ((i=${#ver1[@]}; i<${#ver2[@]}; i++)); do
        ver1[i]=0
    done

    for ((i=0; i<${#ver1[@]}; i++)); do
        if [[ -z ${ver2[i]} ]]; then
            # fill empty fields in ver2 with zeros
            ver2[i]=0
        fi
        if ((10#${ver1[i]} > 10#${ver2[i]})); then
            return 1
        fi
        if ((10#${ver1[i]} < 10#${ver2[i]})); then
            return 2
        fi
    done

    return 0
}

version_ge() {
    version_compare "$1" "$2"
    case $? in
        0) return 0;;
        1) return 0;;
        2) return 1;;
    esac
}

# Settings
INSTALL_DIR="$(abspath "$1")"
BUILD_DIR="$(abspath "$2")"
IKOS_SRC_DIR="$(abspath "$3")"
OS=$(uname)
ARCH=$(uname -m)
NPROC=$(nproc)

# Check IKOS source directory
if [ ! -d "$IKOS_SRC_DIR" ]; then
    echo "$progname: error: $3 is not a directory."
    exit 1
elif [ ! -f "$IKOS_SRC_DIR/CMakeLists.txt" ] || [ ! -d "$IKOS_SRC_DIR/frontends" ] || [ ! -d "$IKOS_SRC_DIR/abs-repr" ] || [ ! -d "$IKOS_SRC_DIR/analyzer" ]; then
    echo "$progname: error: directory $3 does not contain IKOS source code."
    exit 1
fi

set -e

mkdir -p "$INSTALL_DIR"
mkdir -p "$BUILD_DIR"
mkdir -p "$BUILD_DIR/download"

#
# CMake
#

echo "[-] Looking for CMake"
INSTALL_CMAKE=0

if ! command -v cmake >/dev/null 2>&1; then
    INSTALL_CMAKE=1
    echo "[-] CMake not found."
else
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo "[-] CMake $CMAKE_VERSION found."

    if ! version_ge "$CMAKE_VERSION" "2.8.12.2"; then
        INSTALL_CMAKE=1
    fi
fi

if (( INSTALL_CMAKE )); then
    # Check if CMake is already installed
    if [ -x "$INSTALL_DIR/cmake-3.5.2/bin/cmake" ]; then
        echo "[-] Using already built cmake 3.5.2"
    else
        echo "[-] Downloading cmake 3.5.2"
        cd "$BUILD_DIR/download"
        download "https://cmake.org/files/v3.5/cmake-3.5.2-$OS-$ARCH.tar.gz"

        echo "[-] Installing cmake 3.5.2"
        tar xf "cmake-3.5.2-$OS-$ARCH.tar.gz"
        mv "cmake-3.5.2-$OS-$ARCH" "$INSTALL_DIR/cmake-3.5.2"
    fi

    PATH="$INSTALL_DIR/cmake-3.5.2/bin:$PATH"
fi

#
# GCC
#

echo "[-] Looking for a compiler"
INSTALL_GCC=0

if ! command -v gcc >/dev/null 2>&1; then
    echo "[-] GCC not found."

    if ! command -v clang >/dev/null 2>&1; then
        echo "[-] Clang not found. Abort."
        echo "Please install a compiler first."
        exit 1
    else
        CLANG_VERSION=$(clang --version | head -n1 | cut -d' ' -f3)
        echo "[-] Clang $CLANG_VERSION found."

        if version_ge "$CLANG_VERSION" "3.4"; then
            export CC=$(command -v clang)
            export CXX=$(command -v clang++)
        else
            INSTALL_GCC=1
        fi
    fi
else
    GCC_VERSION=$(gcc --version | head -n1 | cut -d' ' -f3)
    echo "[-] GCC $GCC_VERSION found."

    if version_ge "$GCC_VERSION" "5.0"; then
        export CC=$(command -v gcc)
        export CXX=$(command -v g++)
    else
        INSTALL_GCC=1
    fi
fi

if (( INSTALL_GCC )); then
    if [ -x "$INSTALL_DIR/gcc-6.1.0/bin/gcc" ]; then
        echo "[-] Using already built gcc 6.1.0"
    else
        echo "[-] Downloading GCC 6.1.0"
        cd "$BUILD_DIR/download"
        download "ftp://gcc.gnu.org/pub/gcc/releases/gcc-6.1.0/gcc-6.1.0.tar.gz"
        download "ftp://gcc.gnu.org/pub/gcc/infrastructure/gmp-6.1.0.tar.bz2"
        download "ftp://gcc.gnu.org/pub/gcc/infrastructure/mpfr-3.1.4.tar.bz2"
        download "ftp://gcc.gnu.org/pub/gcc/infrastructure/mpc-1.0.3.tar.gz"
        download "ftp://gcc.gnu.org/pub/gcc/infrastructure/isl-0.16.1.tar.bz2"

        echo "[-] Extracting GCC sources"
        rm -rf "$BUILD_DIR/gcc-6.1.0"
        tar xf "gcc-6.1.0.tar.gz"
        mv "gcc-6.1.0" "$BUILD_DIR/gcc-6.1.0"
        tar xf "gmp-6.1.0.tar.bz2"
        mv "gmp-6.1.0" "$BUILD_DIR/gcc-6.1.0/gmp"
        tar xf "mpfr-3.1.4.tar.bz2"
        mv "mpfr-3.1.4" "$BUILD_DIR/gcc-6.1.0/mpfr"
        tar xf "mpc-1.0.3.tar.gz"
        mv "mpc-1.0.3" "$BUILD_DIR/gcc-6.1.0/mpc"
        tar xf "isl-0.16.1.tar.bz2"
        mv "isl-0.16.1" "$BUILD_DIR/gcc-6.1.0/isl"

        echo "[-] Building GCC 6.1.0"
        cd "$BUILD_DIR/gcc-6.1.0"
        mkdir build
        cd build
        ../configure \
            --prefix="$INSTALL_DIR/gcc-6.1.0" \
            --enable-languages=c,c++ \
            --enable-threads=posix \
            --enable-threads=posix \
            --enable-__cxa_atexit \
            --with-isl \
            --disable-multilib

        make -j$NPROC
        make install
    fi

    PATH="$INSTALL_DIR/gcc-6.1.0/bin:$PATH"
    export CC="$INSTALL_DIR/gcc-6.1.0/bin/gcc"
    export CXX="$INSTALL_DIR/gcc-6.1.0/bin/g++"
    if [ -d "$INSTALL_DIR/gcc-6.1.0/lib64" ]; then
        LIB_DIR="lib64"
    else
        LIB_DIR="lib"
    fi
    export LDFLAGS="-L$INSTALL_DIR/gcc-6.1.0/$LIB_DIR -Wl,-rpath,$INSTALL_DIR/gcc-6.1.0/$LIB_DIR"
fi

#
# LLVM
#

echo "[-] Looking for LLVM"
INSTALL_LLVM=0

if (( INSTALL_GCC )); then
    echo "[-] We have to build LLVM with the previously built gcc"
    INSTALL_LLVM=1
elif ! command -v opt >/dev/null 2>&1; then
    echo "[-] LLVM not found."
    INSTALL_LLVM=1
else
    LLVM_VERSION=$(opt --version | head -n2 | tail -n1 | cut -d' ' -f5)
    echo "[-] LLVM $LLVM_VERSION found."

    if ! version_ge "$LLVM_VERSION" "3.7"; then
        INSTALL_LLVM=1
    fi
fi

if (( INSTALL_LLVM )); then
    if [ -x "$INSTALL_DIR/llvm-3.7.1/bin/opt" ]; then
        echo "[-] Using already built llvm 3.7.1"
    else
        echo "[-] Downloading LLVM 3.7.1"

        if ! command -v svn >/dev/null 2>&1; then
            echo "$progname: error: could not find svn."
            exit 1
        fi

        rm -rf "$BUILD_DIR/llvm-3.7.1"
        svn co "http://llvm.org/svn/llvm-project/llvm/tags/RELEASE_371/final" "$BUILD_DIR/llvm-3.7.1" >/dev/null
        cd "$BUILD_DIR/llvm-3.7.1"
        svn co "http://llvm.org/svn/llvm-project/cfe/tags/RELEASE_371/final" tools/clang >/dev/null
        svn co "http://llvm.org/svn/llvm-project/compiler-rt/tags/RELEASE_371/final" projects/compiler-rt >/dev/null

        echo "[-] Building LLVM 3.7.1"
        mkdir build
        cd build
        cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR/llvm-3.7.1" \
            -DCMAKE_BUILD_TYPE=Release \
            -DLLVM_TARGETS_TO_BUILD=X86 \
            -DLLVM_ENABLE_RTTI=ON \
            -DLLVM_ENABLE_EH=ON \
            -DLLVM_ENABLE_CXX1Y=ON ..

        make -j$NPROC
        make install
    fi

    PATH="$INSTALL_DIR/llvm-3.7.1/bin:$PATH"
fi

#
# Boost
#

echo "[-] Looking for boost"
INSTALL_BOOST=0

if (( INSTALL_GCC )); then
    echo "[-] We have to build boost with the previously built gcc"
    INSTALL_BOOST=1
elif [ ! -d /usr/include/boost ]; then
    echo "[-] Boost not found."
    INSTALL_BOOST=1
fi

if (( INSTALL_BOOST )); then
    if [ -d "$INSTALL_DIR/boost-1.60.0/include/boost" ]; then
        echo "[-] Using already built boost 1.60.0"
    else
        echo "[-] Downloading Boost 1.60.0"
        cd "$BUILD_DIR/download"
        download "https://sourceforge.net/projects/boost/files/boost/1.60.0/boost_1_60_0.tar.gz"

        echo "[-] Extracting Boost sources"
        tar xf "boost_1_60_0.tar.gz"
        mv "boost_1_60_0" "$BUILD_DIR/boost-1.60.0"

        echo "[-] Building Boost 1.60.0"
        cd "$BUILD_DIR/boost-1.60.0"
        ./bootstrap.sh --prefix="$INSTALL_DIR/boost-1.60.0" --with-libraries=program_options
        ./b2 install
    fi

    export BOOST_ROOT="$INSTALL_DIR/boost-1.60.0"
    export LDFLAGS="$LDFLAGS -L$INSTALL_DIR/boost-1.60.0/lib -Wl,-rpath,$INSTALL_DIR/boost-1.60.0/lib"
fi

#
# IKOS
#

if [ -x "$INSTALL_DIR/ikos-1.0.0/bin/arbos" ]; then
    echo "[-] IKOS already built"
else
    echo "[-] Building IKOS 1.0.0"
    mkdir -p "$BUILD_DIR/ikos-1.0.0"
    cd "$BUILD_DIR/ikos-1.0.0"
    cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR/ikos-1.0.0" "$IKOS_SRC_DIR"
    make -j$NPROC
    make install
fi

echo "[-] IKOS has been successfully installed"
echo ""
echo "Set the following environment variables to use IKOS (consider adding this in your .bashrc):"
echo ""
echo "export IKOS_INSTALL=\"$INSTALL_DIR/ikos-1.0.0\""
echo "PATH=\"$INSTALL_DIR/ikos-1.0.0/bin:$INSTALL_DIR/llvm-3.7.1/bin:\$PATH\""
echo ""
echo "If you want to launch IKOS tests, move under $BUILD_DIR/ikos-1.0.0 and follow the instructions from README.md"
echo ""
echo "You can remove the build directory $BUILD_DIR if you don't want to run the tests anymore."

set +e
