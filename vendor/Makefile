lua_cflags:=-std=c99 -Wall -Wextra -pedantic -g -O3 -DLUA_USE_POSIX $(CFLAGS)
lua_objects:=$(patsubst lua/%.c,lua/%.o,$(filter-out lua/lua.c, $(wildcard lua/*.c)))

all: uthash/src/utlist.h pcre/.libs/libpcre.a lua/liblua5.3.a termbox/libtermbox.a

uthash/src/utlist.h:
	command -v git && git submodule update --init

pcre/.libs/libpcre.a:
	cd pcre && ./autogen.sh && ./configure && $(MAKE)

lua/liblua5.3.a: $(lua_objects)
	$(AR) rcs $@ $(lua_objects)

termbox/libtermbox.a:
	$(MAKE) -C termbox

$(lua_objects): %.o: %.c
	$(CC) -c $(lua_cflags) $< -o $@

$(termbox_objects): %.o: %.c
	$(CC) -c $(termbox_cflags) $< -o $@

clean:
	rm -f lua/liblua5.3.a $(lua_objects)
	rm -f pcre/config.h.in~
	-$(MAKE) -C termbox clean
	-$(MAKE) -C pcre clean

.PHONY: all clean
