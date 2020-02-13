# Calcul

This project aimed to be a simple calculator with potentialy huge number.

```C++
Hi!
Just type 'q' to leave the program

>>> 34 * (2 + 3) - 18
INT 0x98 = 152
  
>>> 0x100 * 2x1000
INT 0x800 = 2048
  
>>> 5x132 ^ 2x101010
BIG [29 bytes] 0x5919417cd6a11dbdf2f413657bbc03bc842ac76ef3932640000000000
```



## Number

By default the numbers are in decimal, but it handles different bases.

You can prefix number with the base using **one** decimal digit followed by **x** and then the **core** number

For instance, the number 14​ in different bases looks like: 

- hexadecimal        `0xe` or `0xE`
- unary                    `1x00000000000000`
- binary                   `2x1110`
- quaternary          `4x32`
- quinary                `5x24`
- octal                     `8x16`

The hexadecimal is a special case, but the other bases follow this regular expression: with `b` in [1, 9], `bx [1-(b-1)]+ .? [1-(b-1)]* `

> float numbers aren't implemented yet



## Flow explanations

The `main.c` file calls a `console`, which ask for a **input line** and print the result or the `error` message.

> In `main`, I also define the `log` verbosity thanks to the macros `RELEASE` and `LOG_LEVEL` 

The input line is sent to the `lexer` which performs a **lexical** analysis.

Then, the parser convertes the expression and check the **syntax** as much as possible to return a expressive error.

Next, the expression is sent to `eval` that first performs a **syntaxe transforamation** with `shunting_yard`, from an array of *token* to a `stack` ordered in Reverse Polish Notation ([RPN](https://en.wikipedia.org/wiki/Reverse_Polish_notation)).

> The syntaxe transformation uses the [Shunting yard](https://en.wikipedia.org/wiki/Shunting-yard_algorithm) algorithm, with [unary operator](https://stackoverflow.com/questions/16425571/unary-minus-in-shunting-yard-expression-parser). Also, here is my source for [operator precedence](https://en.wikipedia.org/wiki/Order_of_operations#Programming_languages)
>

Then, durring the evaluation, operands are converted to a `struct number` which **stores the value** as a `long` or a `struct big_int` (the `struct big_int` should manage **operation** on huge size integers). That's the responsability of `struct number` to check overflow on `long` and switch to `struct big_int` when needed.

The result of the evaluation is simply a `struct number`.



## Compile

The classic `make` command compiles the `main` executable to run.

> The makefile is the result of how I work on this project (I compile with clang-902.0.39.1)

### Run (release)

To compile a **release version** (without inner `assert` and `log`), just type

```makefile
make RELEASE=yes
```

I also use the command `rlwrap` around the executable `rlwrap ./main` to be more user friendly. Try the command for that

```makefile
make run
```

### Test

Run `make tst` to compiles `test` and executes tests over the whole project.

Otherwise, I typed expression in the console to see if it works as I wanted. Also, I can set the `LOG_LEVEL` to control the verbosity during test.

### Log

The release version doesn't output log, but, the debug version has level `LOG_WARN` by default.

The log level can be chosen by setting the variable `LOG_LEVEL`

```makefile
make run LOG_LEVEL=LOG_INFO
```

among the values `LOG_TRACE`, `LOG_DEBUG`, `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`, `LOG_FATAL`

> `LOG_TRACE` follows `malloc` and `free` 
>
> `LOG_DEBUG` follows some function and give debuging values
>
> `LOG_INFO` explicites operation

### Clean

Clean the whole project (in `src/`) using

```makefile
make clean
```

Executables included (`main` and `test`) with the command

```makefile
make mrproper
```



## Restriction

For now, I restrict the math expression to a sentence with few operators (`+`, `-`, `*`, `^`), [numbers](#number), parenthesis <u>but</u> no function, neither variable.

That's possible that the code doesn't work on big endian architecture (because of function like `add_big` in `big_int.c` for instance).



## Add an operator

Here are all the changes I made to add the operator `^` (exponentiation). I think knowing how to add a operation should help to understand the sources.

1. `lexer.c` should recognize the charactere `'^'` as a `SYMBOL`, so I add it in the function `try_symbol`

2. add the new operator `POW` in the `enum token_type`. 

   ```C
   enum token_type {
   	POW
   };
   ```
   and add the `case` in `print_token` for convenience.

3. Then in `parser.c`, first implement the convertion between the symbol to the effective operator for `lexer_to_perser`, it's in `convert_token`

   ```c
   case SYMBOL:
   			if (strncmp(token.str, "^", token.len) == 0) {
   				return POW;
   			}
   ```

   Second, for `parser_check_syntax` add the token in the `BINARY` macro

   ```c
   #define BINARY(op)   ( .... || ((op) == POW))
   ```

4. In `shunting_yard.c` set it in `preced`, `assoc` and the `case` in `shunting_yard_wye`. 

5. Finally in `eval.c`, in `eval_token` again add the `case POW`.

   ```C
   case POW: {
   			struct number res = binary_op(operands, number_pow);
   			stack_push(operands, &res);
   			....
   		}
   ```

   Where `number_pow` is an implementation of the operation on two `struct number`.
   
   > Well, implement the operation might be the hardest part...