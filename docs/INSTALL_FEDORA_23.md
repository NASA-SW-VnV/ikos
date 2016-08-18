INSTALL IKOS DEPENDENCIES ON FEDORA 23
======================================

Here are the steps to install the required dependencies of IKOS on **Fedora 23**.

First, make sure your system is up-to-date:

```
$ sudo dnf update
```

Now, install the following packages:

```
$ sudo dnf install make cmake python gmp-devel sqlite-devel zlib-devel libedit-devel boost-devel gcc gcc-c++ llvm llvm-devel llvm-static clang
```

Unfortunately, there is a problem in the **llvm-devel** Fedora package. We need to update a path in LLVMConfig.cmake. Run the following command:

```
$ sudo sed -i 's#set(_LLVM_LIBRARY_DIR .*)#set(_LLVM_LIBRARY_DIR "${LLVM_INSTALL_PREFIX}/lib64/llvm")#' /usr/share/llvm/cmake/LLVMConfig.cmake
```

You are now ready to build IKOS. Go to the next section of the `README.md`.
