CFLAGS = -Wall -g
TARGET = GBEmu
SOURCE_DIR = source
CC := gcc

SOURCES = $(wildcard $(SOURCE_DIR)/*.c)

all: $(TARGET)

$(TARGET): $(SOURCES)
		$(CC) $(CFLAGS) -o $@ $^
