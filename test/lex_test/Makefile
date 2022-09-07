mini_test:
	lex mini_test.l
	gcc lex.yy.c -o mini_test -ll

tokenizer_test.cpp: mini_test.l
	flex -o tokenizer_test.cpp mini_test.l

clean:
	-rm -f tokenizer_test.cpp
	-rm -f mini_test
	-rm -f lex.yy.c