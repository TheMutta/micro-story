all: engine editor

editor:
	g++ editor/*.cpp -I/usr/include/SDL2 -lSDL2 -o story_editor

engine:
	g++ engine/*.cpp -o story_engine

