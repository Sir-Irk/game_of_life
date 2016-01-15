CC = clang++
CFLAGS= -DGAME_DEBUG=1 -DUSING_GLM=1 -Wall -Wno-switch -Wno-missing-braces -Wno-unused-variable -Wno-unused-function -Wno-c++11-compat-deprecated-writable-strings -Wno-c++11-extensions
LFLAGS= -lstdc++ -lGL -lopenal -lalut -lm -lGLEW -lGLU -lglfw 
INPUT=linux_platform_layer
OUTPUT=platform_layer
OUTPUT_GAME=game

# -lSOIL -lSDL2_image

all: gameDLL platform_layer 

gameDLL:
	$(CC) $(CFLAGS) -o $(OUTPUT_GAME).so game.cpp $(LFLAGS) -shared -ldl -fPIC
platform_layer:
	$(CC) $(CFLAGS) -o $(OUTPUT) $(INPUT).cpp $(LFLAGS) -ldl -fPIC


clean:
	rm -f $(OUTPUT)
	rm -f $(OUTPUT_GAME).so
	
