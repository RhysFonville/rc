<h1>My terrible language</h1>
<h2>A custom compiler for a language that prioritizes typing as little as possible.</h2>


<h3>To install/compile/run:</h3>

1. Clone the repository into the directory of your choice (For example, ~/Documents/)
```
git clone git@github.com:RhysFonville/rc.git
```
The rc directory that is cloned is where the executable for the compiler will be. (./rc/rc)

I recommend keeping this directory in the directory that you will be working and writing your rc code in.

2. Compile and run your code 

<i>Linux</i>
```
rc/rc mycode.txt
```

<h3>Syntax of the language</h3>

<h4>Preprocessor macros</h4>

You can include another file's code using the `inc` macro. It is used like this:
```
%inc "hello.txt"
```
The `%` token represents a macro, like `#` in C and C++. `inc` is for including, like `include` in C and C++. And `"hello.txt"` is the file to include.

<h4>Variables</h4>

The types include:
<ul>
    <li>lng (long)</li>
    <li>int (integer)</li>
    <li>sht (short)</li>
    <li>ch (character)</li>
</ul>

To declare a variable, the syntax goes as follows:
```
int i 35
```
This declares an integer named `i` with an initial value of 35. You are not required to give an initial value.

Place type qualifiers in front of the type. like `const ^^int p &x`.

<h4>Pointers</h4>

Defining a pointer looks something like this:
```
^^int my_int_pointer &some_regular_variable
```
This example defines a pointer to an integer (address of `some_regular_variable`).

Honestly, the type doesn't really restrict you. It will probably just lead to some weird behaviour. So, you can assign pointers to anything, really.

Note that with strings, if you want to set a character pointer to a string, you still have to add the address token in front of the string literal. Like this:
```
^^ch my_string &"Hello world!"
```
This might be changed later, idk

<h4>Functions</h4>

A function declaration looks like this:
```
#my_example_function
```

To return, use `#>`. The value to return goes immediately after this token. All functions must return some value, no matter what. Use a closing brace to end the function. There is no need to use an opening brace.

An example function could look like:
```
#my_example_function
    int a 2+3
    #> a
}
```

Calling a function is simple. You simply just write the function's name.
```
#my_example_function
    int a 2+3
    #> a
}

#main
    my_example_function
    #> 0
}
```

As you can see, you must have a main function.

<h4>Conditionals</h4>

The syntax of a conditional is as follows:
```
a == b ?
    // do stuff
}
```
In this example, stuff will only be done if a is equal to b. The conditional expressions are just the same as *most* other languages (ahem, lua. Like why ~=? It doesn't make sense).

The `?` shows that this is a conditional, and `a == b` is the condition. Close the condition with `}`. There is no need to use an opening brace to begin the conditional.

Rc also supports else statements. It's syntax is as follows:
```
a == b ?
    // do stuff
} ??
    // do stuff if condition is not true
}
```
In this example, stuff will be done if a is equal to b. But, if a is *not* equal to b, other stuff will be done.

If-else statments are also easy and canbe do like so:
```
a == b ?
    // do stuff
} ?? a > b ?
    // do stuff if second condition is true
}
```

<h4>While loops</h4>

While loops are defined with the `*?` token and closed off with a curly brace. It looks very similar to an if statement.
```
a == b *?
    // do stuff
}
```
In this example, "// do stuff" will be looped until a == is false (a != b).

<h4>Base Functions</h4>

Base functions (or syscalls) can be easily identified in rc since it starts with a `>`. For example, the performing the `write` syscall looks like this:
```
>w 1 str 10
```
`w` stands for write, `1` is to specify stdout, str is the string variable assumed to be defined earlier in the program, and 10 is the number of characters to write.

Rc does not support all syscalls. Only sys_write, sys_read, and sys_exit. Their "names" are 'w' (as seen in the example), 'r', and 'e', respectively.

<h3>Tests</h3>

The tests folder includes a `testrc` executable and a directory called `tests` that includes the programs to compile and run. Adding a new test program and running `testrc` should run your tests. Or, if you are using my makefile, just run `make test`.

To create a new test, create a directory in the `tests` folder and add the rc program in that new folder. Something like this:

```
cd rc/tests/tests   // Go into test folder
mkdir my-test       // Create directory for new test
cd my-test          // Go to new directory
nvim main.txt       // Write test, using neovim as an example
cd ../../..         // Go back to rc folder
make test           // Run test(s)
```

<h3>Hello World!</h3>
Although there are already simple tests and this README, I believe it would be best to provide a hello world.

```
#main
    >w 1 &"Hello world!" 12 // Execute write syscall to stdout of string "Hello world!". Write 12 characters. (Print "Hello world!".)
    >e 0                    // Exit with code 0
    #> 0                    // Return 0
}                           // Technically not needed.
```
