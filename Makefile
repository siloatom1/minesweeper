CC=g++

CCFLAGS=-Wall -Wpedantic -Weffc++ -std=gnu++17 -g -pg -O0 -IC:\MinGW\include\SDL2

LDFLAGS=-LC:\MinGW\lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

FILES=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

OBJS=$(FILES)


compile: $(OBJS)
	$(CC) $(CCFLAGS) -o run $^ $(LDFLAGS)

clean:
	rm *.o ./run

%.o: %.cpp %.h
	$(CC) $(CCFLAGS) -c $<

%.o: %.cpp
	$(CC) $(CCFLAGS) -c $<
