CC      := cc
TARGET  := graph
SRC     := graph.c

WARN    := -Wall -Wextra -Wpedantic -Wshadow -Wconversion
STD     := -std=c11

# Default: debug build with sanitizers
CFLAGS  := $(STD) $(WARN) -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS := -fsanitize=address,undefined

.PHONY: all debug release run valgrind clean

all: debug

debug: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

# Optimized build, no sanitizers
release: CFLAGS  := $(STD) $(WARN) -O2 -DNDEBUG
release: LDFLAGS :=
release: clean $(TARGET)

# Run from the project root
run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
