.PHONY: all clean clean_all

CC = ./musl/bin/gcc
CXX = ./musl/bin/g++
CFLAGS = -static -Wall -Wextra -Werror -Wpedantic

DEBUG ?= no
if ${DEBUG}=Y || ${DEBUG}=y || ${DEBUG}=yes
    CFLAGS += -g -O0
endif

SRCDIR = ./src
BINDIR = ./bin
OBJDIR = ./obj
MUSLDIR = ./musl
LIBUSBDIR = ./libusb

all: $(BINDIR)/powerdmx

$(BINDIR)/powerdmx: $(OBJDIR)/program.o $(OBJDIR)/cliengine.o $(OBJDIR)/powerdmx.o | $(BINDIR) $(MUSLDIR) $(LIBUSBDIR)
	$(CXX) $(CFLAGS) -o $@ $^ "$(LIBUSBDIR)/lib/libusb-1.0.a"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR) $(MUSLDIR) $(LIBUSBDIR)
	$(CXX) $(CFLAGS) -I"$(LIBUSBDIR)/include" -c $< -o $@

$(BINDIR) $(OBJDIR):
	mkdir -p $@

$(MUSLDIR):
	@echo Downloading Musl...
	@curl https://musl.cc/x86_64-linux-musl-native.tgz -o ./musl.tgz --progress-bar
	@echo Extracting Musl...
	@tar -xvf ./musl.tgz 1>/dev/null 2>&1
	@rm ./musl.tgz
	@mv ./x86_64-linux-musl-native ./musl

test:
	@rm -rf ./libusb_git
	@echo Cloning LibUsb...
	@git clone https://github.com/libusb/libusb.git ./libusb_git
	@echo Building LibUsb...
	@cd libusb_git && ./bootstrap.sh
	@cd libusb_git && ./configure --host=x86_64-linux-musl --enable-static --disable-shared --disable-udev --prefix="$(realpath ../libusb)"
	@$(MAKE) -C ./libusb_git -j$(nproc)
	@$(MAKE) -C ./libusb_git install

clean:
	rm -rf $(OBJDIR)
	rm -rf $(BINDIR)

clean_all:
	rm -rf $(MUSLDIR)
	rm -rf $(LIBUSBDIR)
	@$(MAKE) clean --no-print-directory