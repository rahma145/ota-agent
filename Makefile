#CC       ?= gcc
#PKG_CONFIG ?= pkg-config

# --- Bibliothèques ---
LIBS ?= libsystemd libcurl libcjson libcrypto
# LIBS   ?= libsystemd libcurl libcjson libcrypto

# --- Options ---
CFLAGS  += -Wall -Wextra -g -Iinclude \
           $(shell $(PKG_CONFIG) --cflags $(LIBS))
#LDLIBS  += $(shell $(PKG_CONFIG) --libs $(LIBS))

# --- Fichiers ---
SRC_AGENT := $(filter-out src/healthcheck.c, $(wildcard src/*.c))
OBJ_AGENT := $(SRC_AGENT:src/%.c=build/%.o)
DEP_AGENT := $(OBJ_AGENT:.o=.d)
BIN_AGENT := ota-agent

SRC_HEALTHCHECK := $(filter-out src/main.c, $(wildcard src/*.c))
OBJ_HEALTHCHECK := $(SRC_HEALTHCHECK:src/%.c=build/%.o)
DEP_HEALTHCHECK := $(OBJ_HEALTHCHECK:.o=.d)
BIN_HEALTHCHECK := ota-healthcheck

PREFIX ?= /usr
DESTDIR ?=

all: agent healthcheck

# --- Règles ---
agent: $(BIN_AGENT)

$(BIN_AGENT): $(OBJ_AGENT)
	$(CC) $(LDFLAGS) -o $@ $(OBJ_AGENT) -lsystemd -lcurl -lcjson -lcrypto

healthcheck: $(BIN_HEALTHCHECK)

$(BIN_HEALTHCHECK): $(OBJ_HEALTHCHECK)
	$(CC) $(LDFLAGS) -o $@ $(OBJ_HEALTHCHECK) -lsystemd -lcurl -lcjson -lcrypto

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

build:
	mkdir -p build

install: $(BIN_AGENT) $(BIN_HEALTHCHECK)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 0755 $(BIN_AGENT) $(DESTDIR)$(PREFIX)/bin/$(BIN_AGENT)
	install -m 0755 $(BIN_HEALTHCHECK) $(DESTDIR)$(PREFIX)/bin/$(BIN_HEALTHCHECK)

clean:
	rm -rf build $(BIN_AGENT) $(BIN_HEALTHCHECK)

-include $(DEP_AGENT) $(DEP_HEALTHCHECK)

.PHONY: all install clean
