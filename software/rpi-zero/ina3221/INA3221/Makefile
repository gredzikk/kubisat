# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -O2

# Target executable
TARGET = read

# Source files
SRCS = read.cpp INA3221.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Include directories
INCLUDES = -I.

# Libraries
LIBS = -lwiringPi

# Build target
all: $(TARGET)

# Link object files to create the executable
$(TARGET): $(OBJS)
    $(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Compile source files into object files
%.o: %.cpp
    $(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean up build files
clean:
    rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean