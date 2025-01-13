CPP = g++
C = gcc

LDFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf
CFLAGS = -Wall -c -std=c89
CXXFLAGS = -Wall -c -Og -std=c++17   # C++ flags for compiling my C++ code.

BIN1 = main

all: $(BIN1)

$(BIN1): main.o kiss_widgets.o kiss_draw.o kiss_general.o kiss_posix.o
	$(CPP) $^ $(LDFLAGS) -o $@

main.o: ../../src/main.cpp
	$(CPP) $(CXXFLAGS) $^ -o $@

kiss_widgets.o: kiss_widgets.c
	$(C) $(CFLAGS) $^ -o $@

kiss_draw.o: kiss_draw.c
	$(C) $(CFLAGS) $^ -o $@

kiss_general.o: kiss_general.c
	$(C) $(CFLAGS) $^ -o $@

kiss_posix.o: kiss_posix.c
	$(C) $(CFLAGS) $^ -o $@

clean:
	rm *.o && rm $(BIN1)

