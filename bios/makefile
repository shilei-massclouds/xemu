.PHONY: all clean
.SUFFIXES: .bin .elf

CC = riscv64-linux-gnu-gcc
COPY = riscv64-linux-gnu-objcopy
CFLAGS = -no-pie -Wall -nostdlib -D__ASSEMBLY__ -fno-omit-frame-pointer \
		 -fno-optimize-sibling-calls -mno-save-restore -mstrict-align
LDFLAGS = -Wl,--build-id=none

OBJS = $(subst .S,.bin, $(wildcard *.S))

%.elf:%.S
	$(CC) -Ttext=0x1000 $(CFLAGS) ./$^ -o ./$@ $(LDFLAGS)

%.bin:%.elf
	$(COPY) -O binary ./$^ ./$@

all:$(OBJS)
	dtc -I dts -O dtb ./virt.dts > ./virt.dtb
	cp ./bios.bin ../image/
	cp ./virt.dtb ../image/

clean:
	rm -rf ./*.elf ./*.bin ./*.dtb
