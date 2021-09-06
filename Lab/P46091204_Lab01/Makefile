CC = gcc
LDLIBS += -lmraa
CFLAGS += -Wall -Wextra -O2

OUT_DIR = bin
LIBS = code/util.h
ENTRIES = $(wildcard code/*.c)
BINS = $(addprefix $(OUT_DIR)/, $(ENTRIES:code/%.c=%))
DST_HOST = em_up
DST_DIR = /home/embedded

.PHONY: hooks clean all

all: $(BINS)

hooks:
	cp -p scripts/git-hooks/* .git/hooks/

clean:
	rm -rf $(OUT_DIR)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(OUT_DIR)/%: code/%.c $(LIBS) | $(OUT_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LIBS) $(LDLIBS)
