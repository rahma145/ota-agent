#CC       ?= gcc
#PKG_CONFIG ?= pkg-config

# --- Bibliothèques ---
LIBS ?= libsystemd libcurl
# LIBS   ?= libsystemd libcurl libcjson libcrypto

# --- Options ---
CFLAGS  += -Wall -Wextra -g -Iinclude \
           $(shell $(PKG_CONFIG) --cflags $(LIBS))
#LDLIBS  += $(shell $(PKG_CONFIG) --libs $(LIBS))

# --- Fichiers ---
SRC := $(wildcard src/*.c)
OBJ := $(SRC:src/%.c=build/%.o)
DEP := $(OBJ:.o=.d)
BIN := ota-agent

PREFIX ?= /usr
DESTDIR ?=

# --- Règles ---
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) -lsystemd

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

build:
	mkdir -p build

install: $(BIN)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 0755 $(BIN) $(DESTDIR)$(PREFIX)/bin/$(BIN)

clean:
	rm -rf build $(BIN)

-include $(DEP)

.PHONY: all install clean
