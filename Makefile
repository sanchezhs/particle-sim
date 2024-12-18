PLATFORM ?= LINUX

CC = gcc
EMCC = emcc

SOURCES = src/main.c src/gui.c src/simulation.c src/cJSON.c
BUILD_DIR = build

ifeq ($(PLATFORM), PLATFORM_WEB)
    BUILD_WEB_ASYNCIFY    ?= FALSE
    BUILD_WEB_HEAP_SIZE   ?= 128MB
    BUILD_WEB_STACK_SIZE  ?= 1MB
    BUILD_WEB_ASYNCIFY_STACK_SIZE ?= 1048576
    BUILD_WEB_SHELL ?= src/minshell.html

    CC = $(EMCC)
    CFLAGS = -Wall -Wextra -pedantic -std=c99 -g \
             -I./external/raylib/src \
             -I./external/raylib/src/external \
             -I./external/raylib/src/libraylib.a \
             -I~/emsdk/upstream/emscripten/system/include
    LDFLAGS = -L./external/raylib/src -lraylib \
              -s ASSERTIONS=1 \
              -s USE_GLFW=3 \
              --profiling \
              -s USE_WEBGL2=1 \
              -s ALLOW_MEMORY_GROWTH=1 \
              -D PLATFORM_WEB \
              --shell-file $(BUILD_WEB_SHELL) \
              --preload-file resources \
              -s WASM=1 \
              -s MODULARIZE=1 \
              -sEXPORTED_RUNTIME_METHODS=ccall,cwrap \
              -s EXPORTED_FUNCTIONS=_main,_apply_config,_on_resize
              
    TARGET = $(BUILD_DIR)/index.html

else ifeq ($(PLATFORM), LINUX)
    CFLAGS = -Wall -Wextra -pedantic -std=c99 -ggdb
    LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    TARGET = $(BUILD_DIR)/simulation
else ifeq ($(PLATFORM), WINDOWS)
    CFLAGS = -Wall -Wextra -pedantic -std=c99 -ggdb
    LDFLAGS = -lraylib -lgdi32 -lopengl32 -lm
    TARGET = $(BUILD_DIR)/simulation.exe
else
    $(error Unknown PLATFORM=$(PLATFORM))
endif

.PHONY: all clean

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)
