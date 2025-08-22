CXX = clang++
CC  = clang

CXXFLAGS = -std=c++17 -O2 -Iinclude -I/opt/homebrew/include
CFLAGS   = -O2 -Iinclude -I/opt/homebrew/include

LDFLAGS  = -L/opt/homebrew/lib -lglfw -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL

SRC_CPP = src/main.cpp src/cube.cpp src/shader_utils.cpp src/character.cpp 
SRC_C   = src/glad.c
OBJ     = $(SRC_CPP:.cpp=.o) $(SRC_C:.c=.o)
BIN     = humangl

all: $(BIN)

re: clean all

$(BIN): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BIN) $(OBJ)
