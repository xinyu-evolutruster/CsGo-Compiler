# mini_test: *.c *.h
#	gcc -o mini_test *.c -lm

# *.c *.h: mini.l mini.y
#	lex mini.l
#	yacc -dv mini.y

CXXFLAGS = -std=c++14 $(NO_WARNING)

all: mini_test

parser.cpp: mini.y
	bison -d -o parser.cpp mini.y

parser.hpp: parser.cpp

tokenizer.cpp: mini.l
	flex -o tokenizer.cpp mini.l

%.o: %.cpp
	g++ -c $(CXXFLAGS) -g -o $@ $<

mini_test: parser.o tokenizer.o
	g++ -o $@ parser.o tokenizer.o

clean:
	rm -f parser.cpp parser.hpp tokenizer.cpp
	rm -f *.o
run:
	./mini_test