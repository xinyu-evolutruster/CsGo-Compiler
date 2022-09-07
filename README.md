# CsGo-Compiler
CsGo compiler compiles a self-designed language with features in both C and Golang.

## Grammar Rules
The EBNF rules are as follows:
~~~
1. program -> declaration-list
2. declaration-list -> declaration-list declaration | declaration
3. declaration -> var-declaration | fun-declaration
4. var-declaration -> type-specifier ID; | type-specifier ID [NUM];
5. type-specifier -> int | float | void
6. fun-declaration -> func ID(params) (params) compound-stmt
7. params -> param-list | void
8. param-list -> param-list, param | param
9. param -> ID type-specifier | ID [] type-specifier 
10. compound-stmt -> {local-declarations statement-list}
11. local-declarations -> local-declarations var-declaration | empty
12. statement-list -> statement-list statement | empty
13. statement -> expression-stmt | compound-stmt | selection-stmt
                 | iteration-stmt | return-stmt  | function-stmt
14. expression-stmt -> expression;|;
15. selection-stmt -> if (simple-expression) statement
                      | if (simple-expression) statement else statement
16. iteration-stmt -> while (simple-expression) statement
17. return-stmt -> return; | return return expression;
18. expression -> var-list = expression-list
19. expressoion-list -> expression-list, simple-expression | simple-expression
20. var-list = var-list, var | var | _	
21. simple-expression -> additive-expression ...
22. relop -> <= | < | > | >= | == | !=
23. additive-expression -> additive-expression addop term | term
24. addop -> + | -
25. term -> term mulop factor | factor
26. mulop -> * | /
27. factor -> (expression) | var | call | NUM
28. call -> ID (args)
29. args -> arg-list | empty
30. arg-list -> arg-list, expression | expression
~~~

check them in `ebnf.md`.
