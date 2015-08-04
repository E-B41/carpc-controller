CC=g++ 
CPPFLAGS= -lstdc++ -lpthread -lrt -lwiringPi
SRC = $(wildcard *.cpp)
DEPS = $(wildcard *.h)
OBJ = $(patsubst %.cpp,%.o,$(SRC))

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< -I. $(CPPFLAGS)

carpc-controller: $(OBJ)
	$(CC) -o $@ $^ -I. $(CPPFLAGS)

clean:
	rm -f *.o
