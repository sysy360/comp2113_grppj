# ─────────────────────────────────────────────────────────────
#  Makefile – Maze Adventure
#  Compiles all .cpp files and links into 'maze'
# ─────────────────────────────────────────────────────────────

CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
TARGET   = maze
SRCS     = main.cpp map.cpp virus.cpp game.cpp score.cpp
OBJS     = $(SRCS:.cpp=.o)
HEADERS  = globals.h map.h virus.h game.h score.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
