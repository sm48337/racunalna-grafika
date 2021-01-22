LIBS_INCLUDE_PATH = ../libs
CFLAGS = -g -std=c++17
OPT_CFLAGS = -O3 -std=c++17
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -I$(LIBS_INCLUDE_PATH)

all: BSpline

BSpline: main.cpp
		g++ $(CFLAGS) -o BSpline main.cpp $(LDFLAGS)

release:
		g++ $(OPT_CFLAGS) -O3 -o BSpline main.cpp $(LDFLAGS)

.PHONY: test clean

run: BSpline
	./BSpline

clean:
	rm -f BSpline
