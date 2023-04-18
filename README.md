
# Mirror256 Hash Function

## DISCLAIMER: Experimental, do not use in production!

## Introduction

This repository contains a C++ implementation of the Mirror Hash algorithm, along with a command line interface for testing the algorithm.

## Building the Command Line Interface

To build the command line interface, follow these steps:

1. Clone the repository to your local machine.
2. Navigate to the `mirror-hash-cpp` directory.
3. Run the following command to build the `mirror256` binary:

   ```
   make
   ```

## Using the Command Line Interface

The `mirror256` binary accepts input from either a file or from `stdin`. If an input filename is provided, the contents of the file will be used as input to the hash algorithm. If no input filename is provided, the input will be read from `stdin`.

To run an example use:

```
./mirror -e
```

To use the command line interface, run the following command:

```
cat [input_file] | ./mirror256
```

Replace `[input_file]` with the path to the input file (optional).

```bash
cat mirror256.cpp | ./mirror256
Hash: 90d49764cf0300c03820d30ba040096f55495b48166e7e160049517353111b47
```

The hash output will be printed to `stdout`.
