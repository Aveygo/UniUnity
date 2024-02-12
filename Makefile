CC = gcc
CFLAGS = -g -Wall -Werror -fsanitize=address -std=c99
#-ansi #<-- way too many for loops for this to be considered a reasonable change

SRCS = main.c global/global.c utils/utils.c student/student.c group/group.c solver/solver.c compress/compress.c writer/writer.c headless/headless.c menu/menu.c 
TARGET = main

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)
	@rm -f $(SRCS:.c=.o)  # Remove object files after linking

clean:
	rm -f $(TARGET)