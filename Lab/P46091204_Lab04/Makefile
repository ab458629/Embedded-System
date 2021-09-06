CC = gcc
LDLIBS += -lmraa
CFLAGS += -Wall -Wextra -O2

OUT_DIR = bin
LIBS = src/util.h
ENTRIES = $(wildcard src/*.c)
BINS = $(addprefix $(OUT_DIR)/, $(ENTRIES:src/%.c=%))
DST_HOST = em_up
DST_DIR = /home/embedded

.PHONY: clean all

all: $(BINS)
clean:
	rm -rf $(OUT_DIR)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OUT_DIR)/%: src/%.c $(LIBS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS) $(LDLIBS)
