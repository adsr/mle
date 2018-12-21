lua_cflags:=-std=c99 -Wall -Wextra -pedantic -g -O3 -DLUA_USE_POSIX $(CFLAGS)
lua_objects:=$(patsubst lua/%.c,lua/%.o,$(filter-out lua/lua.c, $(wildcard lua/*.c)))
termbox_cflags:=-std=c99 -Wall -Wextra -pedantic -Wno-unused-result -g -O3 -D_XOPEN_SOURCE $(CFLAGS)
termbox_objects:=$(patsubst termbox/src/%.c,termbox/src/%.o,$(wildcard termbox/src/*.c))

all: ./pcre/.libs/libpcre.a ./lua/liblua5.3.a ./termbox/src/libtermbox.a vendor/uthash/.git

vendor/uthash/.git:
	git submodule update --init

./pcre/.libs/libpcre.a:
	cd pcre && ./autogen.sh && ./configure && $(MAKE)

./lua/liblua5.3.a: $(lua_objects)
	$(AR) rcs $@ $(lua_objects)

./termbox/src/libtermbox.a: $(termbox_objects)
	$(AR) rcs $@ $(termbox_objects)

$(lua_objects): %.o: %.c
	$(CC) -c $(lua_cflags) $< -o $@

$(termbox_objects): %.o: %.c
	$(CC) -c $(termbox_cflags) $< -o $@

clean:
	rm -f ./lua/liblua.a $(lua_objects)
	rm -f ./termbox/src/libtermbox.a $(termbox_objects)
	$(MAKE) -C pcre clean

.PHONY: all clean
