# README

This project aimed to be a simple calculator with potentialy huge number.



## Flow explanations

The `main.c` file calls a *console*, which ask for a input line and print the result or an error.

The input line is sent to the *parser* which perform a lexical analysis (*lexer*).
Then a syntaxe transforamation (*check_syntax*) to a Reverse Polish Notation ([RPN](https://en.wikipedia.org/wiki/Reverse_Polish_notation)).

> The syntaxe transformation is using [Shunting yard algorithm](https://en.wikipedia.org/wiki/Shunting-yard_algorithm), with an additional step to use `-` as an [unary operator](https://stackoverflow.com/questions/16425571/unary-minus-in-shunting-yard-expression-parser)
>
> Also, here is my source for [operator precedence](https://en.wikipedia.org/wiki/Order_of_operations#Programming_languages)


The result is sent to something that evaluate the expression without error.

### Restriction

For now, I reduce the math expression to a sentence with few operators (`+`, `-`, `*`), parenthesis and no function.

It's possible that the code doesn't work on big endian architecture (for instance because of fuction `add_big` in `big_int.c`).



## File by file

This section justify the responsability of those independent module.

- **big_int:** manage huge integer operations
- **console**
- **eval**
- **lexer:** splits an input string into an array of `lexer_token`. The lexicon is very small because it focuses on the abstraction on the string.
- **log**
- **number:** abstract the computed numbers. The numerical value is store in a `long` as long as possible, then convert into a `big_int` when needed.  
- **parser:** splits an input string into an array of `parser_token` (based on `lexer`), and then checks the syntax as much as possible.
- **shunting_yard:** changes an array of `parser_token` into a `stack`of token to process the expression (Reverse Polish Notation).
- **stack:** stack implementation



## Compile

The classic `make` command compiles the `main` executable. To run it, try `make run`

> The make commands are the result of how I work on this project

### Release

To compile a **release version** (without inner `assert` and `log`), just type

```bash
make RELEASE=yes
```

> maybe `make mrproper` before

### Test

Run `make tst` to compile the `test` executable which runs tests over the whole project

Otherwise, I typed expression in the console to see if it works as I wanted. For instance, an expression designed to challenge the parser is

```c
12 + -(13.0 * +var_1 - max(-1, 2))
```

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

The log level can be chosen by the setting the variable `LOG_LEVEL`

```makefile
make run LOG_LEVEL=LOG_INFO
```



## About the expression

[precedence](https://en.wikipedia.org/wiki/Order_of_operations#Programming_languages)

[associative property](https://en.wikipedia.org/wiki/Associative_property)

[base convertisor](https://www.convzone.com/hex-to-decimal/) (hex -> dec)

### Add operator



## TODO list

- [x] write a `test` target in makefile
- [x] write a target with cflag `-DNDEBUG` to compile a realease
- [x] Refactore `parser`, have a proper `shuning yard` module for instance
- [x] `big_int_add` handle sign, as well as `big_int_mul`
- [x] `console` should catch interrupt (done with `rlwrap`)
- [ ] write test for `shunting_yard` and the whole `parser`
- [ ] write **Add operator** section in readme

