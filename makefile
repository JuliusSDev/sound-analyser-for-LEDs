CC := g++
CFLAGS := -g -Wall -pedantic

# Header files in the src directory
HDR := $(wildcard src/*.hpp)

# Source files in the src directory
SRCS := $(wildcard src/*.cpp)

# List your libs -l* files:
libs = -lfftw3 -lfftw3_omp -lportaudio -fopenmp -lbluetooth

includePath = $(shell pwd)/include/

libsPath = $(shell pwd)/include/

# Specify name of your program:
executable = target/SoundAnalyser

$(executable): $(SRCS) $(HDR)
	$(CC) $(CFLAGS) -I$(includePath) -L$(libsPath) $(libs) $(SRCS) -o $(executable)

.PHONY: clean
clean:
	rm $(executable)

.PHONY: check
check: $(executable)
	valgrind --leak-check=full --track-origins=yes ./$(executable)

.PHONY: run
run:
	./target/SoundAnalyser
