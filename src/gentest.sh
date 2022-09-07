make clean
make
./mini ../test/course_assist/c_assist.gc
clang ../test/course_assist/c_assist.o -o ../test/course_assist/c_assist.out
./mini ../test/matrix/matrix.gc
clang ../test/matrix/matrix.o -o ../test/matrix/matrix.out
./mini ../test/qsort/qsort.gc
clang ../test/qsort/qsort.o -o ../test/qsort/qsort.out
echo "\e[1;33;41m test auto-advisor \e[0m"
cd ../test/course_assist
./linux-amd64 ./c_assist.out
echo "\e[1;33;41m test matrix \e[0m"
cd ../matrix
./linux-amd64 ./matrix.out
echo "\e[1;33;41m test qsort \e[0m"
cd ../qsort
./linux-amd64 ./qsort.out
