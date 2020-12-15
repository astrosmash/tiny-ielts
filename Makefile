LLC ?= llc
CLANG ?= clang

THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
TC_SRC_DIR ?= $(THIS_DIR)tc

USERSPACE_SRC_DIR ?= $(THIS_DIR)userspace


CLANG_FLAGS_COMMON += -D__NR_CPUS__=$(NPROC) -O2
CLANG_INCLUDES_COMMON += -I. -I$(USERSPACE_SRC_DIR)/libbpf/include

CLANG_FLAGS_USERSPACE += -L$(USERSPACE_SRC_DIR) $(CLANG_FLAGS_COMMON) -Wall -Wextra -Wshadow -Wpedantic
CLANG_INCLUDES_USERSPACE += $(CLANG_INCLUDES_COMMON)

NPROC := $(shell nproc)
CURRENT_KERNEL := $(shell uname -r | sed "s/[-].*$\//")
USERSPACE_OBJECT = userspace_cli

all: verify_cmds verify_target_bpf installation_doc userspace_cli

installation_doc:
	@echo "the following packages are required (Ubuntu example): libc6-dev-i386 llvm clang libelf-dev\n\n";

	@echo "for tc object, you will need bpf_api.h and bpf_elf.h in $(TC_SRC_DIR)/iproute_include corresponding to your kernel version $(CURRENT_KERNEL):";
	@echo "# wget https://mirrors.edge.kernel.org/pub/linux/utils/net/iproute2/iproute2-$(CURRENT_KERNEL).tar.gz -O- | tar xzf - iproute2-$(CURRENT_KERNEL)/include/bpf_api.h iproute2-$(CURRENT_KERNEL)/include/bpf_elf.h";
	@echo "# mkdir $(TC_SRC_DIR)/iproute_include; cp -a iproute2-$(CURRENT_KERNEL)/include/* $(TC_SRC_DIR)/iproute_include; rm -rf iproute2-*\n\n";

	@echo "for userspace object, you will need libbpf library corresponding to your kernel version $(CURRENT_KERNEL):";
	@echo "assuming that you've downloaded kernel sources from kernel.org into /opt:";
	@echo "# cd /opt/linux-4.$(CURRENT_KERNEL)/tools/lib/bpf; make";
	@echo "# cp libbpf.so libbpf.a $(USERSPACE_SRC_DIR)\n\n";

# Verify LLVM compiler tools are available and bpf target is supported by llc
.PHONY: verify_cmds verify_target_bpf $(CLANG) $(LLC)

verify_cmds: $(CLANG) $(LLC)
	@for TOOL in $^ ; do \
		if ! (which -- "$${TOOL}" > /dev/null 2>&1); then \
			echo "*** ERROR: Cannot find LLVM tool $${TOOL}" ;\
			exit 1; \
		else true; fi; \
	done

verify_target_bpf: verify_cmds
	@if ! (${LLC} -march=bpf -mattr=help > /dev/null 2>&1); then \
		echo "*** ERROR: LLVM (${LLC}) does not support 'bpf' target" ;\
		echo "   NOTICE: LLVM version >= 3.7.1 required" ;\
		exit 2; \
	else true; fi

userspace_cli: $(CLANG) $(LLC)
	$(CLANG) $(CLANG_FLAGS_USERSPACE) $(CLANG_INCLUDES_USERSPACE) $(USERSPACE_SRC_DIR)/userspace.c -o $(USERSPACE_OBJECT)

clean:
	rm -f $(USERSPACE_OBJECT)
