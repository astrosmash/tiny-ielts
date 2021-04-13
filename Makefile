CLANG ?= clang
CLANG-FORMAT ?= clang-format
CPPCHECK ?= cppcheck
SCAN-BUILD ?= scan-build
GCC ?= gcc
VALGRIND ?= valgrind

THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
PROJECT_SRC_DIR ?= $(THIS_DIR)project

CLANG_FLAGS_COMMON += -D__NR_CPUS__=$(NPROC) -O2
CLANG_INCLUDES_COMMON += -I. -I$(PROJECT_SRC_DIR)/libbpf/include

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Darwin)
GTK = gtk+-3.0
else
GTK = gtk+-3.0-1
endif

GTK_CFLAGS := $(shell pkg-config --cflags $(GTK))
GTK_INCLUDE := $(shell pkg-config --libs $(GTK))

CLANG_FLAGS_PROJECT += -L$(PROJECT_SRC_DIR) $(CLANG_FLAGS_COMMON) $(GTK_CFLAGS) -lcurl -Wall -Wextra -Wshadow -Wpedantic
CLANG_INCLUDES_PROJECT += $(CLANG_INCLUDES_COMMON) $(GTK_INCLUDE)

NPROC := $(shell nproc)
PROJECT_FILES := $(shell ls $(PROJECT_SRC_DIR))
CURRENT_KERNEL := $(shell uname -r | sed "s/[-].*$\//")
PROJECT_OBJECT := output_bin


# all: installation_doc sanitize format build

all: build

installation_doc:
	echo "hi $(CURRENT_KERNEL)\n";

# Verify that compiler and tools are available
ifeq ($(UNAME_S), Darwin)
.PHONY: verify_cmds $(CLANG) $(CPPCHECK) $(CLANG-FORMAT) $(GCC)
verify_cmds: $(CLANG) $(CPPCHECK) $(CLANG-FORMAT) $(GCC)
else
.PHONY: verify_cmds $(CLANG) $(CPPCHECK) $(CLANG-FORMAT) $(GCC) $(SCAN-BUILD) $(VALGRIND)
verify_cmds: $(CLANG) $(CPPCHECK) $(CLANG-FORMAT) $(GCC) $(SCAN-BUILD) $(VALGRIND)
endif
	@for TOOL in $^ ; do \
		if ! (which -- "$${TOOL}" > /dev/null 2>&1); then \
			echo "*** ERROR: Cannot find tool $${TOOL}" ;\
			exit 1; \
		else true; fi; \
	done

sanitize: verify_cmds
	@if ! (printf "[1/6 clang] \n\n" && ${CLANG} --analyze $(PROJECT_SRC_DIR)/main.c) || \
				! (printf "[2/6 clang] \n\n" && ${CLANG} -fsanitize=memory,undefined,integer -fsanitize-memory-track-origins=2 \
						-fno-omit-frame-pointer -fsanitize-memory-use-after-dtor \
						-g -O2 $(PROJECT_SRC_DIR)/main.c) || \
				! (printf "[3/6 clang] \n\n" && ${CLANG} -fsanitize=address -fno-omit-frame-pointer -g -O2 $(PROJECT_SRC_DIR)/main.c) || \
				! (printf "[4/6 scan-build] \n\n" && ${SCAN-BUILD} -V gcc -c $(PROJECT_SRC_DIR)/main.c 2>&1 | grep -vE '/usr/bin/scan-build line 1860.') || \
				! (printf "[5/6 cppcheck] \n\n" && ${CPPCHECK} $(PROJECT_SRC_DIR) -j $(NPROC) --template=gcc --language=c --std=c99 --std=posix \
						--platform=unix64 --enable=all --inconclusive --report-progress --verbose --force --check-library \
						-I/usr/include -I/usr/include/linux -I/usr/lib/gcc/x86_64-linux-gnu/7/include 2>&1 | grep -vE '__|There is no|equals another one|Checking|information:|scope') || \
				! (printf "[6/6 gcc] \n\n" && ${GCC} -Wall -Wextra -Wpedantic -Wshadow -Wno-unused-but-set-variable $(PROJECT_SRC_DIR)/main.c); then \
		echo "*** ERROR: sanitize failed" ;\
		exit 2; \
	else true; fi

format: verify_cmds
	@for FILE in $(PROJECT_FILES) ; do \
		if ! ($(CLANG-FORMAT) -style='Webkit' $(PROJECT_SRC_DIR)/"$${FILE}" > /tmp/"$${FILE}" && diff $(PROJECT_SRC_DIR)/"$${FILE}" /tmp/"$${FILE}"); then \
			echo "*** ERROR: consider $(CLANG-FORMAT) -verbose -style='Webkit' project/"$${FILE}" > /tmp/"$${FILE}" && mv /tmp/"$${FILE}" project/"$${FILE}"" ;\
			exit 3; \
		else true; fi; \
	done

mem_sanitize: verify_cmds
	@if ! ($(VALGRIND) -v --track-origins=yes --leak-check=full --show-leak-kinds=all  $(THIS_DIR)$(PROJECT_OBJECT) -c $(THIS_DIR)config.conf -tw 222) ; then \
                echo "*** ERROR: valgrind failed" ;\
                exit 2; \
        else true; fi

build: $(CLANG)
	$(CLANG) $(CLANG_FLAGS_PROJECT) $(CLANG_INCLUDES_PROJECT) $(PROJECT_SRC_DIR)/main.c -o $(PROJECT_OBJECT)

clean:
	rm -f $(PROJECT_OBJECT) a.out main.o main.plist

