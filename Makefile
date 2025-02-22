PREFIX := /usr/local
CFLAGS := -O3 -Wall -Wmissing-prototypes -I ./inc
SOURCES := ./src/acidwarp.c ./src/palinit.c ./src/rolnfade.c ./src/display.c ./src/handy.c
IMGGEN_SOURCES := ./src/bit_map.c ./src/lut.c ./src/img_int.c ./src/img_float.c
OBJECTS = $(SOURCES:%.c=%.o)

ifeq ($(GL),1)
# OpenGL ES / WebGL builds require SDL 2
SDL := 2
CFLAGS += -DWITH_GL
else
SDL := 1
endif

ifdef EMMAKEN_COMPILER
# Using emmake make to build using Emscripten
PLATFORM := Emscripten
else
PLATFORM := $(shell uname)
endif

ifeq ($(SDL),2)
SDL_CONFIG := sdl2-config
else
SDL_CONFIG := sdl-config
endif

ifeq ($(PLATFORM),Emscripten)
WORKER_SOURCES := $(IMGGEN_SOURCES) worker.c
WORKER_LDFLAGS := $(CFLAGS) -s BUILD_AS_WORKER=1
WORKER_OBJECTS := $(WORKER_SOURCES:%.c=%.o)
SOURCES += useworker.c
all: worker.js
worker.js: $(WORKER_OBJECTS)
	@rm -f $@
	$(LINK) $(WORKER_LDFLAGS) $^ -o $@
CC = emcc
ifeq ($(SDL),2)
CFLAGS += -s USE_SDL=2
endif
TARGET := acidwarp.html
$(TARGET): template.html
LDFLAGS := $(CFLAGS) --shell-file template.html -s TOTAL_MEMORY=33554432

else

SOURCES += $(IMGGEN_SOURCES) ./src/draw.c

CONVERTEXISTS := $(shell command -v convert > /dev/null 2>&1 && \
                   convert -version 2> /dev/null | grep ImageMagick)
ifdef CONVERTEXISTS
CFLAGS += -DADDICON
SOURCES += ./src/acid_ico.c
endif

CFLAGS += $(shell $(SDL_CONFIG) --cflags)
LIBS := $(shell $(SDL_CONFIG) --libs) -lm
ifeq ($(GL),1)
ifeq ($(PLATFORM),Darwin)
LIBS += -framework OpenGL
else ifneq (,$(findstring CYGWIN,$(PLATFORM)))
LIBS += -lglew32 -lopengl32
else
LIBS += -lGL
endif
endif

ifneq (,$(findstring CYGWIN,$(PLATFORM)))
EXESUFFIX = .exe
CC := i686-w64-mingw32-gcc
OBJECTS += acid_res.o
endif

LDFLAGS = $(CFLAGS)
TARGET = acidwarp$(EXESUFFIX)
endif

LINK = $(CC)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@rm -f $(TARGET)
	$(LINK) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

acid_ico.o: ./src/acid_ico.c
handy.o: ./src/handy.c ./inc/handy.h
acidwarp.o: ./src/acidwarp.c ./inc/handy.h ./inc/acidwarp.h ./inc/rolnfade.h ./inc/display.h
bit_map.o: ./src/bit_map.c ./inc/handy.h ./inc/bit_map.h
display.o: ./src/display.c ./inc/acidwarp.h ./inc/display.h
draw.o: ./src/draw.c ./inc/acidwarp.h ./inc/bit_map.h ./inc/display.h
img_float.o: ./src/img_float.c ./inc/acidwarp.h ./src/gen_img.c
img_int.o: ./src/img_int.c ./inc/acidwarp.h ./inc/lut.h ./src/gen_img.c
lut.o: ./src/lut.c ./inc/handy.h ./inc/lut.h
palinit.o: ./src/palinit.c ./inc/acidwarp.h ./inc/palinit.h
rolnfade.o: ./src/rolnfade.c ./inc/acidwarp.h ./inc/rolnfade.h ./inc/palinit.h ./inc/display.h
ifneq (,$(findstring CYGWIN,$(PLATFORM)))
acidwarp.ico: acidwarp.png
	icotool -c -o $@ $^
acid_res.o: acid_res.rc acidwarp.ico
	windres $< $@
endif
# Using ImageMagick to nearest neighbour resize icon for SDL.
# Without it, you can manually do this in another program.
acidwarp.rgb: acidwarp.png
	convert $< -sample 64x64 $@
acid_ico.c: acidwarp.rgb
	xxd -i $< > $@

.PHONY: clean install uninstall

clean:
	$(RM) *.o $(TARGET) acidwarp.ico acidwarp.rgb acid_ico.c \
          acidwarp.html.mem acidwarp.js worker.js worker.js.mem

install: $(TARGET) acidwarp.png acidwarp.desktop
	install $< $(PREFIX)/bin
	xdg-icon-resource install --novendor --context apps \
	                          --size 256 acidwarp.png acidwarp
	xdg-desktop-menu install --novendor acidwarp.desktop

uninstall:
	rm -f $(PREFIX)/bin/$(TARGET)
	xdg-icon-resource uninstall --novendor --context apps \
	                            --size 256 acidwarp
	xdg-desktop-menu uninstall --novendor acidwarp.desktop
