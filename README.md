# Clang Plugins

A collection of out-of-tree Clang plugins.

* **Out-of-tree** plugins - implemented out of the LLVM/Clang source tree and built against the binary installation
* Based on the latest **LLVM 18** version

### Overview

This project implements custom out-of-tree Clang plugins and LLVM-based command line tools (on the base of these plugins). These plugins are simple but can be used to obfuscate arithmetic operations, automatically add informative comments
to literal arguments in function calls, and enforce LLVM code style by warnings, enhancing code security, readability, and maintainability. 

Mostly, these plugins were implemented to learn more about Clang internals (thanks clang-tutor) and "front-end" of the compilers.


### Plugins

Implemented plugins:

| Name                                             | Description                                                                                       |
|--------------------------------------------------|---------------------------------------------------------------------------------------------------|
| [**RecordDeclarations Counter**](lib/RecordDeclCounter.cpp)       | Counts the number of record declarations (class, struct, and union declarations)                    |
| [**Arguments Commenter**](lib/ArgsCommenter.cpp) | adds comments to literal (int, bool, string, char, float) arguments in each function call       |
| [**Code Obfuscator**]() | obfuscates addition and subtraction arithmetic operations for enhanced security                                              |
| [**Code Style Checker**]() | checks for LLVM coding style and issues warning if not followed                                              |



More plugins are being added along the way.

### Build

To build the project, you first need to intsall and build LLVM/Clang:

`brew install llvm`

Or follow the official tutorial on building from source.

Build all passes:

```bash
mkdir build
CLANG_INSTALL_DIR=<installation/dir/of/llvm>
cmake -DLT_Clang_INSTALL_DIR=$CLANG_INSTALL_DIR -G "Ninja" -B build 
cmkae --build .
```

### Run

```bash
# change PluginName accordingly
clang -cc1 -load <build_dir>/lib/libPluginName.dylib -plugin PluginName input_file.cpp
```

#### Implementation

You can find the implementation of the plugins in .cpp files inside `lib` directory and declaration in .h files insde 'include'.