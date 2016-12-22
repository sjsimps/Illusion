
PROG = illusion

CPP_FLAGS = -O2 -Wall -std=c++11
DEBUG_FLAGS += -g -Wall -std=c++11

SRC =  src/pulseaudio_recorder.cpp 
SRC += src/small_fft.cpp
SRC += src/visualizer.cpp
SRC += src/beat_detector.cpp
SRC += src/main.cpp

SDL =-L/usr/local/lib -lSDL2 -lSDL2_image -lpulse-simple -lpulse -lpthread

.PHONY: all
all: $(PROG)

.PHONY: debug
debug:
	g++ $(DEBUG_FLAGS) $(SRC) ${SDL} -o $(PROG)

.PHONY: clean
clean:
	rm $(PROG)

$(PROG): $(SRC)
	g++ $(CPP_FLAGS) $(SRC) ${SDL} -o $(PROG)
