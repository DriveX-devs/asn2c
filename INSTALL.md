
## Compiling From Sources

### Configure

See [REQUIREMENTS.md](REQUIREMENTS.md) for the complete list of dependencies.

Configure with the default settings:

    test -f configure || autoreconf -iv
    ./configure
    make

Configure with non-standard settings:

asn2c-specific ./configure options include:

  Option                 | Description
------------------------ | ---------------------------------------------------
  --enable-ASN_DEBUG     | produce debug log during `make check` testing
  --enable-code-coverage | whether to enable code coverage support
  --enable-Werror        | abort compilation after any C compiler warning
  --enable-test-Werror   | abort compiling tests after any C compiler warning
  --enable-test-32bit    | enable tests for 32-bit compatibility
  --disable-test-ubsan   | disable Undefined Behavior Sanitizer for tests
  --disable-test-asan    | disable Address Sanitizer for tests
  --enable-test-fuzzer   | enable LLVM LibFuzzer for randomized testing

invoke `./configure --help` for details.

### Build

Build the libraries and the compiler:

    make

Ensure asn2c is still behaving well after compiling on your platform:

    make check

### Install

Install the compiler into a standard location:

    make install
    # Use ./configure --prefix to override install location.

Display the `asn2c` manual page:

    man asn2c

## Quick Usage Guide

For a usage guide and more information please refer to:

 * the [README.md](README.md) file

For a more comprehensive usage guide, you can start from the following documents,
still referred to the original asn1c (but a very good starting point for asn2c):
* the quick start PDF [doc/asn1c-quick.pdf](doc/asn1c-quick.pdf)
* the comprehensive usage documentation [doc/asn1c-usage.pdf](doc/asn1c-usage.pdf)

-- 
Adapted by Francesco Di Gregorio (fra.digregorio.2002@gmail.com), starting from
the original INSTALL.md guide by Lev Walkin (vlm@lionet.info)
