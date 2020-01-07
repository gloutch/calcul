# README

The `main.c` file should call a *console*, which ask for a input line and print result or an error.

The input line is sent to the *parser* which perform a lexical analysis (*lexer*).
Then a syntaxe transforamation (*check_syntax*) into a stack as a "notaion polonaise inversée".


The result is sent to something that evaluate the expression without error.