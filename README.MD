# Program Optimization Lab 2018

Aim of the [Program Optimization Lab 2018](http://www2.in.tum.de/hp/Main?nid=366) 
is to explore the LLVM opt tool and extend it with an advanced Value-Set Analysis
according to [Reps et al.](http://research.cs.wisc.edu/wpis/papers/pepm06.invited.pdf).

A PDF version of the presentation we gave at the end of the course is available [here](https://github.com/peterrum/po-lab-2018/raw/master/documentation/document.pdf).

## Authors (in alphabetic order)

* Julian Erhard
* Jakob Gottfriedsen
* Peter Munch
* Alexander Roschlaub
* Michael B. Schwarz

## Installation

Clone the project into the following folder:
```bash
cd llvm/lib/Transforms
git clone git@github.com:peterrum/po-lab-2018.git ValueSetAnalysis
```

and add the following line to the `CMakeLists.txt`-file:
```bash
add_subdirectory(ValueSetAnalysis)
add_subdirectory(ValueSetAnalysis/tutorial)
add_subdirectory(ValueSetAnalysis/benchmark)
```

Finally run `make` and/or `make install` again.

## How to ...

### ... view IR

Recompile and view output with your favorite text editor: 
```bash
clang -Os -S -emit-llvm hello.c -o hello.ll
```

In the case you have already a .bc-file run the following to get the .ll-file:
```bash
llvm-dis hello-opt.bc
```

### ... use opt
```bash
clang -Os -S -emit-llvm hello.c -o hello.ll
```

```bash
clang -O3 -emit-llvm hello.c -c -o hello.bc
opt -load llvm/lib/llvm-vsa.so -vsapass < hello.bc > /dev/null
```

### ... useful workflow

```bash
clang -O0 -emit-llvm hello.c -Xclang -disable-O0-optnone -c -o hello.bc
opt -mem2reg < hello.bc > hello-opt.bc
opt -load llvm/lib/llvm-vsa.so -vsapass < hello.bc > /dev/null
```

## Useful classes and methods in LLVM

* Write output: errs() << "Hello World!\n"

### llvm::Module
* Iterator over all functions

### llvm::Function
* Iterator over all basic blocks
* getName()

### llvm::BasicBlock
* Iterator over all instructions

### llvm::Instruction
* getOpcodeName()
* getValueID()
* users()

### llvm::InstVisitor<U, V>
* V visit(...)

### llvm::APInt
* Arbitrary precision integer

