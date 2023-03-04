# List your *.h files (if you do not have them in your project then leave the variable "headers" empty):
headers = src/SoundAnalyser.hpp

# List your *.cpp files:
sources = src/SoundAnalyser.cpp src/main.cpp

# List your libs -l* files:
libs = -lfftw3 -lportaudio

# Specify name of your program:
executable = target/SoundAnalyser

$(executable): $(headers) $(sources)
	g++ -g -Wall -pedantic $(libs) $(sources) -o $(executable)

.PHONY: clean
clean:
	rm $(executable)

.PHONY: check
check: $(executable)
	valgrind --leak-check=full --track-origins=yes ./$(executable)

.PHONY: run
run:
	./target/SoundAnalyser
