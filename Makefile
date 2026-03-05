# Compiler
CXX = g++
CXXFLAGS = -Wall -std=c++17 -fopenmp  

# Libraries
LIBS = -lglfw -lGL -lGLU -lglut       

# Source files
SRCS = main.cpp functions.cpp
OBJS = $(SRCS:.cpp=.o)

# Output executable
TARGET = solar_system

# Default target
all: $(TARGET)

# Link object files to create executable (include -fopenmp!)
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(CXXFLAGS) $(LIBS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJS) $(TARGET)
