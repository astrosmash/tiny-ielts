CLANG ?= clang
CPPCHECK ?= cppcheck
SCAN-BUILD ?= scan-build
GCC ?= gcc

THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
USERSPACE_SRC_DIR ?= $(THIS_DIR)userspace

CLANG_FLAGS_COMMON += -D__NR_CPUS__=$(NPROC) -O2
CLANG_INCLUDES_COMMON += -I. -I$(USERSPACE_SRC_DIR)/libbpf/include

CLANG_FLAGS_USERSPACE += -L$(USERSPACE_SRC_DIR) $(CLANG_FLAGS_COMMON) -Wall -Wextra -Wshadow -Wpedantic
CLANG_INCLUDES_USERSPACE += $(CLANG_INCLUDES_COMMON)

NPROC := $(shell nproc)
CURRENT_KERNEL := $(shell uname -r | sed "s/[-].*$\//")
USERSPACE_OBJECT = userspace_cli

all: installation_doc sanitize userspace_cli

installation_doc:
	echo "hi $(CURRENT_KERNEL)\n";

# Verify that compiler and tools are available
.PHONY: verify_cmds $(CLANG) $(CPPCHECK) $(SCAN-BUILD) $(GCC)

verify_cmds: $(CLANG) $(CPPCHECK) $(SCAN-BUILD) $(GCC)
	@for TOOL in $^ ; do \
		if ! (which -- "$${TOOL}" > /dev/null 2>&1); then \
			echo "*** ERROR: Cannot find tool $${TOOL}" ;\
			exit 1; \
		else true; fi; \
	done

sanitize: verify_cmds
	@if ! (printf "[1/6 clang] \n\n" && ${CLANG} --analyze $(USERSPACE_SRC_DIR)/userspace.c) || \
				! (printf "[2/6 clang] \n\n" && ${CLANG} -fsanitize=memory,undefined,integer -fsanitize-memory-track-origins=2 \
						-fno-omit-frame-pointer -fsanitize-memory-use-after-dtor \
						-g -O2 $(USERSPACE_SRC_DIR)/userspace.c) || \
				! (printf "[3/6 clang] \n\n" && ${CLANG} -fsanitize=address -fno-omit-frame-pointer -g -O2 $(USERSPACE_SRC_DIR)/userspace.c) || \
				! (printf "[4/6 scan-build] \n\n" && ${SCAN-BUILD} -V gcc -c $(USERSPACE_SRC_DIR)/userspace.c | grep -vE '/usr/bin/scan-build line 1860.') || \
				! (printf "[5/6 cppcheck] \n\n" && ${CPPCHECK} $(USERSPACE_SRC_DIR) -j $(NPROC) --template=gcc --language=c --std=c99 --std=posix \
						--platform=unix64 --enable=all --inconclusive --report-progress --verbose --force --check-library \
						-I/usr/include -I/usr/include/linux -I/usr/lib/gcc/x86_64-linux-gnu/7/include 2>&1 | grep -vE '__|There is no|equals another one|Checking|information:|scope') || \
				! (printf "[6/6 gcc] \n\n" && ${GCC} -Wall -Wextra -Wpedantic -Wshadow -Wno-unused-but-set-variable $(USERSPACE_SRC_DIR)/userspace.c); then \
		echo "*** ERROR: sanitize failed" ;\
		exit 2; \
	else true; fi

userspace_cli: $(CLANG)
	$(CLANG) $(CLANG_FLAGS_USERSPACE) $(CLANG_INCLUDES_USERSPACE) $(USERSPACE_SRC_DIR)/userspace.c -o $(USERSPACE_OBJECT)

clean:
	rm -f $(USERSPACE_OBJECT)
