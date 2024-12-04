# A Mini-C Compiler

## Rules

### Keyword

`int, return, main, void, if, else, while, continue, break`

### Identifier

Identifiers compliant with the C89 standard `([A-Za-z_][0-9A-Za-z_]*)`

### Constant

Decimal integers, such as `1, 223, 10`, etc

### Assignment operator

`=`

### Operator

**Unary Operator**  

`- ! ~`

**Binary Operator**  

`+ - * / % < <= > >= == != & | ^ && ||`

### Punctuation

`; { } ( )`

### Statement

**Variable Declaration**

```c
int a, b=111, c=1+3;
```

**Assignment Statement**

```c
a = (d+b&1)/(e!=3^b/c&d); 
a = b+c;
```

**Return Statement**

```c
return a+b; 
return func(a);
```

**Function Call**

```c
println_int(a+b);
```

**Conditional Statement**

```c
if ( condition ) { ... }
if ( condition ) { ... } else { ...  }
```

**Loop Statement**

```c
while ( condition ) { ... }
```

**Loop Control Statement**

```c
continue;
break;
```

### Function Definition

**Without Arg(s)**

```c
int func(){...}
void func() {...}
```

**With Arg(s)**

```c
int func(int a, int b){...}
void func(int a, int b){...}
```

### Preset Function

> In addition to custom functions, it is also necessary to support calls to preset functions

**println_int(int a)**

Has the same output as `printf("%d\n", a)` in C

### Need to be able to handle function recursion normally


## Get Started

The following command can generate assembly code using gcc

```bash
gcc -S -masm=intel -fverbose-asm test.c
```

To compile with cmake, execute the following command from the command line:

```bash
mkdir build
cd build
cmake ..
cmake –build .
```

The compilation product is called `build/CompilerlabX`.

---

If you want to execute the assembly code yourself and debug it

X86:

Execute in Linux end point after inserting the code yourself

```bash
gcc -m32 -no-pie <input assembly file> -o <output executable file> ./<output executable file>
```

The output can be observed.

> Note: On some machines, you may need to add i386 architecture packages to perform the above operations correctly. The reference command is as follows

```bash
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install libc6:i386 libstdc++6:i386 gcc-multilib
```

