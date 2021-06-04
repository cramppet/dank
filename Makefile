CC := g++
CFLAGS := -Wall -Wextra -shared -std=c++14 -O3 -fPIC

PYBIND11_INCLUDES := $(shell python3 -m pybind11 --includes)
PYBIND11_LIBS := $(shell python3-config --ldflags)
PYBIND11_SUFFIX := $(shell python3-config --extension-suffix)

SOURCES := $(wildcard dank/*.cpp)
OBJECTS := $(patsubst dank/%.cpp, obj/%.o, ${SOURCES})

all: $(OBJECTS) pybind

pybind:
	$(CC) $(CFLAGS) $(PYBIND11_INCLUDES) $(OBJECTS) $(PYBIND11_LIBS) -o dank$(PYBIND11_SUFFIX)

obj/%.o: dank/%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(PYBIND11_INCLUDES) -c $< -o $@

.PHONY : clean

clean:
	rm -r obj/
	rm dank$(PYBIND11_SUFFIX)
