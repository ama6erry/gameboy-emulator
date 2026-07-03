CFLAGS = -Wall -lm `sdl2-config --cflags --libs` -lSDL2 -lSDL2_mixer -lSDL2_ttf -g
TARGET = GBEmu
SOURCE_DIR = source
CC := gcc

SOURCES = $(wildcard $(SOURCE_DIR)/*.c)

all: $(TARGET)

.PHONY: all

$(TARGET): $(SOURCES)
		$(CC) $(CFLAGS) -o $@ $^
