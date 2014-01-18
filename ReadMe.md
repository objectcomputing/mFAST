
### Introduction

mFAST is an encoding/decoding library for FAST (FIX Adapted for STreaming) protocol written in C++.
For more information, please read the [article](http://objectcomputing.github.io/mFAST/).

### Release Notes

* \[01/18/2014\] Added preliminary FAST 1.2 named enum and boolean type support. Anonymous enum, set, bit group and timestamp types are not implemented yet.
* \[01/12/2014\] Generating `omit_fieldName()` for optional fields in generated group/template classes, `field_mref::as_absent()` member function are removed.
