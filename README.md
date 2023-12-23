# My terrible language
## A custom compiler for a language that prioritizes typing as little as possible.


<h3>To install/compile/run:</h3>

1. Clone the repository into the directory of your choice (For example, ~/Documents/)
```
git clone git@github.com:RhysFonville/rc.git
```
The rc directory that is cloned is where the executable for the compiler will be. (./rc/rc)

I recommend keeping this directory in the directory that you will be working and writing your rc code in.

2. Compile and run your code 
```
rc/rc mycode.txt
```


<h3>Syntax of the language</h3>

<h4>Variables</h4>

The types include:
    - lng (long)
    - int
    - sht (short)
    - ch (character)
    - str (string)
    - nstr (string with no zero byte)

To declare a variable, the syntax goes as follows:
```
int i 35
```
This declares an integer named `i` with an initial value of 35. You are not required to give an initial value.


Note: Strings can only be declared globally.

<h4>Functions</h4>

A function declaration looks like this:
```
#my_example_function
```

And to close the it off, you end it with `#>`. This token is also used as a return statement. This means that all functions must return some value, no matter what.

The value you wish to return goes immediately after the `#>`.

An example function could look like:
```
#my_example_function
    int a 2+3
#> a
```

Calling a function is simple. You simply just write the function's name.
```
#my_example_function
    int a 2+3
#> a

#main
    my_example_function
#> 0
```

As you can see, you must have a main function.

<h4>Conditionals</h4>

Conditionals can only be used when/by calling functions. To best explain what this means, I will show an example.

The syntax of a conditional is as follows:
```
my_example_function ? a == b
```
In this example, my_example will only be called if a is equal to b.

The conditional expressions are just the same as *most* other languages (ahem, lua. Like why ~=? It doesn't make sense).

<h4>Base Functions</h4>

Base functions (or syscalls) can be easily identified in rc since it starts with a `>`. For example, the performing the `write` syscall looks like this:
```
>w 1 str 10
```
`w` stands for write, `1` is to specify stdout, str is the string variable assumed to be defined earlier in the program, and 10 is the number of characters to write.


<h3>Tests</h3>

The tests folder includes a `testrc` executable and a tests folder that includes the programs to compile and run. Adding a new test program and running `testrc` should run your tests. Or, if you are using my makefile, just run `make test`.
