# mFAST [![Actions Status](https://github.com/objectcomputing/mFAST/workflows/main/badge.svg)](https://github.com/objectcomputing/mFAST/actions)

### Introduction


mFAST is a high performance C++ encoding/decoding library for FAST (FIX Adapted for STreaming) protocol.

For more information, please read the [article](http://objectcomputing.github.io/mFAST/).

### Download and Build Intructions

For Unix or macOS with boost library installed on the system paths.

```bash
git clone --recursive https://github.com/objectcomputing/mFAST.git
mkdir mFAST/build
cd mFAST/build
cmake ..
make
```

For detailed build instructions for systems without existing boost library installation or on Windows, please read the [wiki page](https://github.com/objectcomputing/mFAST/wiki/Installation).

### Important Notice

[03/13/2016] mFAST has changed the licence from the LGPL v3 to the BSD 3-clause “New” or “Revised” license.

From now on, mFAST require C++ 11 to compile. You can check out the version with tag name "v1.2.1" if you need a C++ 03 only solution.

mFAST has limited support for FAST protocol version 1.2 now. That includes the new `define` and `type` tags in the FAST XML specification and the new enum/boolean types.
In addition, the generated sequence types support iterators now. However, there is no support for *SET*, *BIT GROUP* and *TIMESTAMP* yet.


If you have used mFAST 1.1 version, please notice there are some backward-incompatible changes. First, to make a existing field absent, please use `msg_mref.omit_fieldName()` instead of `msg_mref.set_fieldName().as_absent()`. Second, if you have developed your own visitor for mFAST cref/mref types, the visitor has to be able to visit the new enum cref/mref types.

#### Using mFAST across the web
* OCI has successfully translated the mFAST source code into asm.js (an efficient subset of JavaScript).
* The asm.js version of mFAST when used with Socket.io enables high volume data exchanges between browsers and servers.
* This is an ideal data format for streaming graphs and feeds to web and mobile clients.
  * It can also go the other direction to stream from web and mobile clients.

