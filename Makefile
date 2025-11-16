SRC_DIR := ./src
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)

CXX := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -O2

tosts: $(SOURCES)
	$(CXX) $(CXXFLAGS) ./src/main.cpp -o $@

clean:
	rm -f tosts

.PHONY: clean