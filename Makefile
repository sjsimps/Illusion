CPP_FLAGS = -O2 -Wall -std=c++11
DEBUG_FLAGS += -g -Wall -std=c++11
PROG = illusion
SRC = pulseaudio_recorder.cpp small_fft.cpp visualizer.cpp beat_detector.cpp main.cpp
SDL =-L/usr/local/lib -lSDL2 -lSDL2_image -lpulse-simple -lpulse -lpthread

.PHONY: all
all: illusion

.PHONY: debug
debug:
	g++ $(DEBUG_FLAGS) $(SRC) ${SDL} -o $(PROG)

.PHONY: clean
clean:
	rm $(PROG)

illusion: $(SRC)
	g++ $(CPP_FLAGS) $(SRC) ${SDL} -o $(PROG)
