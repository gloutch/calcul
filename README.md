# README

This project aimed to be a simple calculator with potentialy huge number.



## Flow explanations

The `main.c` file calls a `console`, which ask for a **input line** and print the result or an error.

> In `main` I also define the `log` verbosity thanks to the defined macros

The input line is sent to the `parser` which performs a **lexical** analysis (`lexer`). Then checks the **syntax** as much as possible to return a expressive error.

Next, the expression is sent to `eval` that first performs a **syntaxe transforamation** with `shunting_yard`, from an array of *token* to a `stack` ordered in Reverse Polish Notation ([RPN](https://en.wikipedia.org/wiki/Reverse_Polish_notation)).

> The syntaxe transformation uses the [Shunting yard](https://en.wikipedia.org/wiki/Shunting-yard_algorithm) algorithm, with [unary operator](https://stackoverflow.com/questions/16425571/unary-minus-in-shunting-yard-expression-parser). Also, here is my source for [operator precedence](https://en.wikipedia.org/wiki/Order_of_operations#Programming_languages)
>

Then, durring the evaluation, operand is convert to a `struct number` which **stores the value** as a `long` or a `struct big_int` (the `struct big_int` should manage **operation** on huge size integers). That's the responsability of `struct number` to check overflow on `long` an switch to `struct big_int` when needed.

The result of the evaluation is simply a `struct number`.

> Sorry about that, but for simplicity I print `struct number` in **hexadecimal**. For now, I use [convzone](https://www.convzone.com/hex-to-decimal/) to come back in **decimal**

### Restriction

For now, I restrict the math expression to a sentence with few operators (`+`, `-`, `*`), parenthesis and no function, neither variable.

It's possible that the code doesn't work on big endian architecture (because of function like `add_big` in `big_int.c` for instance).

### File by file

This section justify the responsability of the independent modules.

- **big_int:** manage huge integer operations.
- **console:** manage input/output of the program.
- **eval:** try to evaluate an expression (no error management yet)
- **lexer:** splits a string into an array of `lexer_token`. The lexicon is very small because it focuses on the abstraction on the string.
- **number:** abstract the computed numbers. The numerical value is store in a `long` as long as possible, then convert into a `big_int` when needed.  
- **parser:** splits an input string into an array of `parser_token` (based on `lexer`), and then checks the syntax as much as possible.
- **shunting_yard:** changes an array of `parser_token` into a `stack`of *token* to process the expression (Reverse Polish Notation).
- **stack:** abstract stack implementation



## Compile

The classic `make` command compiles the `main` executable. To run it, try `make run`

> The makefile is the result of how I work on this project (I compile with clang-902.0.39.1)

### Run (release)

To compile a **release version** (without inner `assert` and `log`), just type

```bash
make RELEASE=yes
```

I also use the command `rlwrap` around the executable `rlwrap ./main` to be more user friendly. Try the command for that

```makefile
make rlwrap
```

### Test

Run `make tst` to compiles `test` and executes tests over the whole project.

Otherwise, I typed expression in the console to see if it works as I wanted. Also, I can set the `LOG_LEVEL` to control the verbosity during test.

### Clean

Clean the whole project using

```bash
make clean
```

Included executables with the command

```bash
make mrproper
```

### Log

The release version doesn't output log, while the debug version has level `LOG_WARN` by default.

The log level can be chosen by setting the variable `LOG_LEVEL`

```makefile
make run LOG_LEVEL=LOG_INFO
```

among the values `LOG_TRACE`, `LOG_DEBUG`, `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`, `LOG_FATAL`