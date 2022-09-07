NAME = mini

LLVM_CONFIG = /usr/lib/llvm-10/bin/llvm-config

NO_WARNING =  -Wno-return-type \
	-Wno-switch \

CXXFLAGS =  `$(LLVM_CONFIG) --cppflags` -std=c++14 $(NO_WARNING)
LDFLAGS = `$(LLVM_CONFIG) --ldflags`
LIBS = `$(LLVM_CONFIG) --libs --system-libs`

OBJS = parser.o tokenizer.o ppMacro.o ast.o CodeGenerator.o

all : $(NAME)

parser.cpp: ${NAME}.y
	bison -d -o parser.cpp ${NAME}.y

parser.hpp: parser.cpp

tokenizer.cpp: ${NAME}.l
	flex -o tokenizer.cpp ${NAME}.l

%.o: %.cpp ppMacro.h ast.h CodeGenerator.h
	g++ -c $(CXXFLAGS) -g -o $@ $< 

$(NAME): $(OBJS)
	g++ -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

.PHONY: clean
clean:
	-rm -f *.o
	-rm -f parser.hpp parser.cpp tokenizer.cpp mini
	-rm -f *.json
	-rm -f *.s
	-rm -f *.ll
	-rm -f *.out
	-rm -f *.gcpp
	-rm -f ../test/*/*.ll
	-rm -f ../test/*/*.o
	-rm -f ../test/*/*.gcpp
	-rm -f ../test/*/*.out
	