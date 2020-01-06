nat: nat.o
	g++ -std=c++17 -O2 nat.o -o nat

nat.o: nat.cpp nat.h
	g++ -std=c++17 -c -O2 nat.cpp -o nat.o

clean:
	rm -f *.o nat
