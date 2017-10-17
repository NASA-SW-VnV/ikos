Install IKOS dependencies on Mac OS X
=====================================

Here are the steps to install the required dependencies of IKOS on **[MAC OS X](https://www.apple.com/macos/high-sierra/)** using **[Homebrew](https://brew.sh/)**.

First, install **Homebrew** by following the instructions at http://brew.sh/

Once you have **Homebrew**, install the following packages:

```
$ brew install cmake gmp sqlite boost llvm@4
```

Now, add the LLVM directory to your `PATH` (consider adding this in your `.bashrc`):

```
$ PATH="$(brew --prefix)/opt/llvm@4/bin:$PATH"
```

You are now ready to build IKOS. Go to the section [Build and Install IKOS](../README.md#build-and-install-ikos) in README.md
