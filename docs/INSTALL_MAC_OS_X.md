INSTALL IKOS DEPENDENCIES ON MAC OS X
=====================================

Here are the steps to install the required dependencies of IKOS on **MAC OS X** using **Homebrew**.

First, install **Homebrew** by following the instructions at http://brew.sh/

Once you have **Homebrew**, install the following packages:

```
$ brew install cmake gmp sqlite boost homebrew/versions/llvm37
```

Now, add the LLVM directory to your `PATH` (consider adding this in your `.bashrc`):

```
$ PATH="$(brew --prefix)/opt/llvm37/lib/llvm-3.7/bin:$PATH"
```

You are now ready to build IKOS. Go to the next section of the `README.md`.
