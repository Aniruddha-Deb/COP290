NN matrix library
- Aniruddha Deb

## Compiling
requires `make`. unzip the folder and run `make` to create the `yourcode.out` executable in the `bin` directory. Note that c++14 is the standard that this compiles against.

NOTE: after realizing that yourcode.out may be in the same file, to prevent automated testcase failure, `make` creates a copy of `yourcode.out` in the parent directory after creating it.

## Testcases
The makefile has an inbuilt testcase runner, that compares output files and program outputs. This was made to make program verification a bit easier. 
- Unzip the testcases and rename the directory to `tests`. Keep the directory in the same folder. 
- run `make test` to run all the tests. It will terminate with "All tests passed successfully", if the program worked correctly.

## folder structure: (generated using tree)

2020CS10869
├── Makefile
├── README.txt
├── yourcode.out
├── bin
│   ├── matcomp
│   ├── matcomp.o
│   ├── matio.o
│   ├── nnmat.o
│   ├── test.o
│   └── yourcode.out
├── src
│   ├── matcomp.cpp
│   ├── matio.cpp
│   ├── matio.hpp
│   ├── nnmat.cpp
│   ├── nnmat.hpp
│   └── test.cpp
└── tests
    ├── README.md
    ├── SUMMARY.txt
    ├── a1a.biasmatrix.txt
    ├── a1a.inputmatrix.txt
    ├── a1a.output.txt
    ├── a1a.outputmatrix.txt
    ├── a1a.weightmatrix.txt
    ├── a2a.inputmatrix.txt
    ├── a2a.output.txt
    ├── a2a.outputmatrix.txt
    ├── a2a.type.txt
    ├── a2b.inputmatrix.txt
    ├── a2b.output.txt
    ├── a2b.outputmatrix.txt
    ├── a2b.type.txt
    ├── a3a.inputmatrix.txt
    ├── a3a.output.txt
    ├── a3a.outputmatrix.txt
    ├── a3a.typeStride.txt
    ├── a3b.inputmatrix.txt
    ├── a3b.output.txt
    ├── a3b.outputmatrix.txt
    ├── a3b.typeStride.txt
    ├── a4a.inputvector.txt
    ├── a4a.output.txt
    ├── a4a.outputvector.txt
    ├── a4a.type.txt
    ├── a4b.inputvector.txt
    ├── a4b.output.txt
    ├── a4b.outputvector.txt
    └── a4b.type.txt