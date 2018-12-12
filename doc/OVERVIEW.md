Overview of the source code
===========================

The following illustrates the content of the root directory:

```
.
├── CMakeLists.txt
├── LICENSE.pdf
├── README.md
├── RELEASE_NOTES.md
├── TROUBLESHOOTING.md
├── analyzer
├── ar
├── cmake
├── core
├── doc
├── frontend
├── script
└── test
```


* [CMakeLists.txt](../CMakeLists.txt) is the root CMake file.

* [LICENSE.pdf](../LICENSE.pdf) contains the NOSA 1.3 license.

* [RELEASE_NOTES.md](../RELEASE_NOTES.md) contains the release notes for the latest versions.

* [TROUBLESHOOTING.md](../TROUBLESHOOTING.md) contains solution for common issues with IKOS.

* [analyzer](../analyzer) contains the implementation of various analyses for specific defect detections. These analyses are implemented on the Abstract Representation and use the fixpoint iterator and abstract domains to perform analysis. More information can be found at [analyzer/README.md](../analyzer/README.md).

* [ar](../ar) contains the implementation of the Abstract Representation, a generic assembly language. More information can be found at [ar/README.md](../ar/README.md).

* [cmake](../cmake) contains CMake files to search for related software libraries.

* [core](../core) contains the implementation of the theory of Abstract Interpretation, which includes the abstract domains, the fixpoint iterator, and various algorithms that support the implementation. More information can be found at [core/README.md](../core/README.md).

* [doc/install](../doc/install) contains installation instructions for specific operating systems.

* [frontend/llvm](../frontend/llvm) contains the implementation of the translation from LLVM to AR. More information can be found at [frontend/llvm/README.md](../frontend/llvm/README.md).

* [script](../script) contains the [bootstrap](../script/bootstrap) script for a rootless installation.

* [test/install](../test/install) contains tests for the installation of IKOS on different platforms.
