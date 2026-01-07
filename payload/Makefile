CC := ./musl/bin/gcc
DEBUG_CFLAGS := -Wall -Wextra -Werror -static -g -O0
RELEASE_CFLAGS := -Wall -Wextra -Werror -static -s -O3
SRCS := $(wildcard ./src/*.c)
DEBUG_OBJS := $(SRCS:./src/%.c=./obj/debug/%.o)
RELEASE_OBJS := $(SRCS:./src/%.c=./obj/release/%.o)
INCLUDES := -I"./lib/libdrm/include" -I"./lib/libdrm/include/libdrm" -I"./lib/libalsa/include"
LIBS := ./lib/libalsa/lib/libasound.a ./lib/libdrm/lib/libdrm.a

./bin/mystery: $(RELEASE_OBJS) ./assets/assets.o | ./musl ./lib/libalsa ./lib/libdrm
	@mkdir -p ./bin
	$(CC) $(RELEASE_CFLAGS) -o $@ $^ $(LIBS)

debug: ./bin/mystery_debug
./bin/mystery_debug: $(DEBUG_OBJS) ./assets/assets.o | ./musl ./lib/libalsa ./lib/libdrm
	@mkdir -p ./bin
	$(CC) $(DEBUG_CFLAGS) -o $@ $^ $(LIBS)

./obj/release/%.o: ./src/%.c ./assets/assets.o | ./musl ./lib/libalsa ./lib/libdrm
	@mkdir -p ./obj/release
	$(CC) $(RELEASE_CFLAGS) $(INCLUDES) -ffile-prefix-map=/important_data/Coding/MysteryMemeware/MysteryMemeOS/payload/src=. -c $< -o $@

./obj/debug/%.o: ./src/%.c ./assets/assets.o | ./musl ./lib/libalsa ./lib/libdrm
	@mkdir -p ./obj/debug
	$(CC) $(DEBUG_CFLAGS) $(INCLUDES) -ffile-prefix-map=/important_data/Coding/MysteryMemeware/MysteryMemeOS/payload/src=. -c $< -o $@

assets:
./assets/assets.o: ./assets/mysteryimage.png ./assets/mysterysong.xm | ./musl
	./assets/precompile_assets.py
	$(CC) $(DEBUG_CFLAGS) -c ./assets/assets.c -o ./assets/assets.o
	@rm ./assets/assets.c

./lib/libalsa: ./musl
	@mkdir -p ./lib
	@echo 'Cloning LibAlsa...'
	@git clone https://github.com/alsa-project/alsa-lib.git ./lib/libalsa_git/ 1>/dev/null 2>&1
	@echo 'Building LibAlsa...'
	@cd ./lib/libalsa_git/ && autoreconf -i 1>/dev/null 2>&1
	@cd ./lib/libalsa_git/ && CC='$(abspath $(CC))' ./configure --host=x86_64-linux-musl --prefix='' --disable-shared --enable-static --disable-python --disable-hwdep --disable-topology --disable-aload --disable-seq --disable-rawmidi --disable-ucm --disable-old-symbols --without-versioned --disable-dlset --with-pthread=yes 1>/dev/null 2>&1
	@cd ./lib/libalsa_git/ && make -j$(shell nproc) 1>/dev/null 2>&1
	@cd ./lib/libalsa_git/ && make DESTDIR='$(abspath ./lib/libalsa)' install 1>/dev/null 2>&1
	@rm -rf ./lib/libalsa_git/ 1>/dev/null 2>&1
	@rm -rf ./lib/libalsa/bin/ 1>/dev/null 2>&1
	@rm -rf ./lib/libalsa/lib/pkgconfig/ 1>/dev/null 2>&1
	@rm -f ./lib/libalsa/lib/libasound.la 1>/dev/null 2>&1
	@rm -rf ./lib/libalsa/share/aclocal/ 1>/dev/null 2>&1

./lib/libdrm: ./musl
	@echo $(abspath $(CC))
	@mkdir -p ./lib
	@echo 'Cloning LibDrm...'
	@git clone https://gitlab.freedesktop.org/mesa/libdrm.git ./lib/libdrm_git
	@echo 'Building LibDrm...'
	@cd ./lib/libdrm_git && CC='$(abspath $(CC))' CFLAGS='-static' meson setup ./ninja --reconfigure --prefix='/' --default-library=static -Dintel=disabled -Dradeon=disabled -Damdgpu=disabled -Dnouveau=disabled -Dvmwgfx=disabled -Detnaviv=disabled -Dfreedreno=disabled -Dexynos=disabled -Domap=disabled -Dtegra=disabled -Dcairo-tests=disabled -Dvalgrind=disabled -Dman-pages=disabled
	@cd ./lib/libdrm_git && ninja -j$(shell nproc) -C ./ninja
	@cd ./lib/libdrm_git && DESTDIR='$(abspath ./lib/libdrm)' ninja -C ./ninja install
	@rm -rf ./lib/libdrm_git/
	@rm -rf ./lib/libdrm/lib/pkgconfig

./musl:
	@echo 'Downloading Musl...'
	@curl https://musl.cc/x86_64-linux-musl-native.tgz -o ./musl.tgz --progress-bar
	@echo 'Extracting Musl...'
	@tar -xvf ./musl.tgz 1>/dev/null 2>&1
	@rm ./musl.tgz 1>/dev/null 2>&1
	@mv ./x86_64-linux-musl-native ./musl 1>/dev/null 2>&1

clean:
	rm -rf ./bin ./obj ./assets/assets.h ./assets/assets.o

cleanall:
	rm -rf ./bin ./obj ./assets/assets.h ./assets/assets.o ./lib ./musl

.PHONY: debug assets clean cleanall