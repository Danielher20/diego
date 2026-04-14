CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -pedantic -O2
SRC := $(wildcard src/*.c)
SRC := $(filter-out src/reset_admin.c,$(SRC))
OBJ := $(SRC:.c=.o)
TARGET := siga

ifeq ($(OS),Windows_NT)
TARGET := siga.exe
LDLIBS += -lws2_32
RM := del /Q
CLEAN_OBJ := $(subst /,\,$(OBJ))
else
RM := rm -f
CLEAN_OBJ := $(OBJ)
endif

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	-$(RM) $(CLEAN_OBJ) $(TARGET)
