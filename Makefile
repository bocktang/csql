# Makefile
CC = gcc
CFLAGS = -Wall -O2 -std=c99 -D_GNU_SOURCE
LDFLAGS = -lm -lreadline
TARGET = csql
# 源文件列表(添加csql.c)
SRCS = src/main.c            \
       src/csql.c            \
       src/lexer/lexer.c     \
       src/parser/parser.c   \
       src/backend/backend.c \
       src/catalog/catalog.c \
       src/utils/utils.c
OBJS = $(SRCS:.c=.o)
INCLUDES = -Iinclude
.PHONY: all clean
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
clean:
	rm -f $(TARGET) $(OBJS)
	rm -f .*.swp *.swp
	rm -f .history
debug: CFLAGS += -g -DDEBUG
debug: clean $(TARGET)
