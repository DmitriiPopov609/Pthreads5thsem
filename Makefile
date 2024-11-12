all: prog

prog: main.o matrix.o jord.o
	g++ main.o matrix.o jord.o -o prog -pthread



main.o: main.cpp matrix.hpp jord.hpp
	g++ -g -Wall -Wextra -Werror -pedantic -std=gnu++0x -O3 -c main.cpp

matrix.o: matrix.cpp matrix.hpp
	g++ -g -Wall -Wextra -Werror -pedantic -std=gnu++0x -O3 -c matrix.cpp

jord.o: jord.cpp jord.hpp
	g++ -g -Wall -Wextra -Werror -pedantic -std=gnu++0x -O3 -c jord.cpp


clean:
	rm -f *.o prog
