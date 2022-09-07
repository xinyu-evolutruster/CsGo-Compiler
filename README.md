# CsGo

## Introduction
The CsGo compiler compiles a self-designed language with features in both C and Golang, using flex, bison and LLVM.

## Features

- C macros (supports variadic macro)
- function calls (supports recursion)
- arrays
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
git clone https://github.com/xinyu-evolutruster/CsGo-Compiler.git
```

Then run

```shell
cd ./src
```
to the main directory. Compile the project by running

```shell
make
# the compiler program is named 'mini'
```
To compile your program, run 

```shell
./mini {filename}.gc
clang {filename}.o -o filename.out
```
## Grammar Rules

variable declaration:
```C
int num;
int arr[100];
```

variable assignment:
```C
num = 1;
i, j, k = 1, 2, 3;
i, j, k = begin, end, arr[begin];
```

function declaration:
```go
// func {func_name}({var_name}{var_type}, ...)(ret {var_type})

func main(void)(ret int);
func qsort(arr[] int, begin int, end int)(void);
```

I/O operations:
```C
scanf("%d", len);
printf("%d\n", arr[i]);
```

conditional statements:
```C
if(i<j){
  arr[i]=arr[j];
}
```

loop statements:
```C
while(i<j){
  arr[i]=key;
  i=i+1;
}
```
macros:
```C
#define ZERO 0
#define index(i,j,N) (i*N+j)
```

To learn more about the CsGo language, check [ebnf.md](./ebnf.md) to see all definitions.

### File Description

- src：where the source code is stored

  - mini.l: written in Flex, performs lexical analysis and generates all the tokens

  - mini.y:
  written in Bison, performs syntactic analysis and generates the AST (Abstract Syntax Tree)

  - ast.h:
  defines all the nodes in the AST

  - ast.cpp: 
    implements the AST
  
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
  - qsort
    - qsort.gc：implements quick sort
    - linux-amd64：the test file
  
  - matrix
    - matrix.gc：implements matrix multiplication
    - linux-amd64：the test file
 
  - course_assist
    - c_assist.gc：implements a program to assist in online course selection
    - linux-amd64：the test file
