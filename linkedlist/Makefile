# Makefile

# Compiler
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2

# Target executable
TARGET = bin

# Source files
SRCS = main.cpp

# Build rule
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Clean rule
clean:
	rm -f $(TARGET)
