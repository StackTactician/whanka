CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fPIE -Wformat -Wformat-security
LDFLAGS = -lm -pie

SRCDIR = src
SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/lexer.c \
          $(SRCDIR)/parser.c \
          $(SRCDIR)/interpreter.c \
          $(SRCDIR)/value.c \
          $(SRCDIR)/env.c \
          $(SRCDIR)/error.c

TARGET = whanka

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
