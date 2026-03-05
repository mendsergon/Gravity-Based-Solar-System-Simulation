# Compiler
CXX = g++
CXXFLAGS = -Wall -std=c++17

# Libraries
LIBS = -lglfw -lGL

# Source files
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

# Output executable
TARGET = solar_system

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)
