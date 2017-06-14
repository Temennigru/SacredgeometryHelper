INPUT := sg.c
TARGET := sg
CC := gcc

.PHONY: clean

$(TARGET): $(INPUT)
	$(CC) -o $@ $<

clean:
	rm $(TARGET)