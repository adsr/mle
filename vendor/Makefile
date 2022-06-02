lua_cflags:=-std=c99 -Wall -Wextra -pedantic -g -O3 -DLUA_USE_POSIX $(CFLAGS)
lua_objects:=$(patsubst lua/%.c,lua/%.o,$(filter-out lua/lua.c lua/onelua.c, $(wildcard lua/*.c)))

all: uthash/src/utlist.h pcre2/.libs/libpcre2-8.a lua/liblua5.4.a

uthash/src/utlist.h:
	command -v git && git submodule update --init

pcre2/.libs/libpcre2-8.a:
	cd pcre2 && ./autogen.sh && ./configure && $(MAKE)

lua/liblua5.4.a: $(lua_objects)
	$(AR) rcs $@ $(lua_objects)

$(lua_objects): %.o: %.c
	$(CC) -c $(lua_cflags) $< -o $@

clean:
	rm -f lua/liblua5.4.a $(lua_objects)
	rm -f pcre2/src/config.h.in~
	-$(MAKE) -C pcre2 clean

.PHONY: all clean
