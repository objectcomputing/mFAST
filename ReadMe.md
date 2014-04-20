# mFAST [![Build Status](https://travis-ci.org/objectcomputing/mFAST.svg?branch=master)](https://travis-ci.org/objectcomputing/mFAST)

### Introduction


mFAST is a high performance C++ encoding/decoding library for FAST (FIX Adapted for STreaming) protocol.

For more information, please read the [article](http://objectcomputing.github.io/mFAST/).

For build instructions, please read the [wiki page](https://github.com/objectcomputing/mFAST/wiki/Installation).

### Important Notice

mFAST has a preliminary support for FAST protocol version 1.2 now. That includes the new `define` and `type` tags in the FAST XML specification and the new enum/boolean types.
In addition, the generated sequence types support iterators now. However, there is no support for *SET*, *BIT GROUP* and *TIMESTAMP* yet.


If you have used mFAST 1.1 version, please notice there are some backward-incompatible changes. First, to make a existing field absent, please use `msg_mref.omit_fieldName()` instead of `msg_mref.set_fieldName().as_absent()`. Second, if you have developed your own visitor for mFAST cref/mref types, the visitor has to be able to visit the new enum cref/mref types.
