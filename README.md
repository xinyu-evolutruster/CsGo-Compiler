# CsGo
The CsGo compiler compiles a self-designed language with features in both C and Golang, using flex, bison and LLVM.
## Introduction


## Grammar Rules

## Features

- C macros
- function calls
- multiple variable assignment
- AST visualization

## Install

- OS：Ubuntu 20.04
- Dependencies
  - Flex 2.6.4
  - Bison 3.5.1 (GNU Bison)
  - LLVM 10.0.0

To install, first download the project by

```shell
$> git clone https://github.com/xinyu-evolutruster/CsGo-Compiler.git
```

Then run

```shell
$> cd ./src
```
to the main directory. Compile the project by running

```shell
$> make
# the compiler program is named 'mini'
```
To compile your program, run 

```shell
$> ./mini {filename}.gc
$> clang {filename}.o -o filename.out
```

### File Description

- src：where the source code is stored

  - mini.l: written in Flex, performs lexical analysis and generates all the tokens

  - mini.y:
  written in Bison, performs syntactic analysis and generates the AST (Abstract Syntax Tree)

  - ast.h:
  defines all the nodes in the AST

  - ast.cpp: 
    implements the AST

  - CodeGenerator.h: 中间代码生成器头文件，定义生成器环境
  
  - CodeGenerator.cpp: generates the intermediate representation
  
  - ObjGenerator.h: header file for object code generation
  
  - ppMacro.h: header file for macro definition
  
  - ppMacro.cpp: the implementation of macros
  
  - gentest.sh: 
  script file used to generate object code and perform all tests

  - tree.json: 
  JSON file generated from the AST structure

  - tree.html：visualization of the AST

- test: stores the test cases
  
  to perform the test, run
  ```shell
  $> ./mini {filename}.gc -o {filename}.out
  $> linux-amd64 {filename}.out
  ```

  or just run the `gentest.sh` file to view all tests results.

  - qsort
    - qsort.gc：implements quick sort
    - linux-amd64：the test file
  
  - matrix
    - matrix.gc：implements matrix multiplication
    - linux-amd64：the test file
 
  - course_assist
    - c_assist.gc：implements a program to assist in online course selection
    - linux-amd64：the test file

### 1.5 运行方式

可使用测试脚本`gentest.sh`运行提供的测试程序

```shell
cd ./src
make
./mini filename.gc
clang filename.o -o filename.out
```

