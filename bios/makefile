.PHONY: all clean
.SUFFIXES: .bin .elf

CC = riscv64-linux-gnu-gcc
COPY = riscv64-linux-gnu-objcopy
CFLAGS = -no-pie -Wall -nostdlib -D__ASSEMBLY__ -fno-omit-frame-pointer \
		 -fno-optimize-sibling-calls -mno-save-restore -mstrict-align
LDFLAGS = -Wl,--build-id=none

%.elf:%.S
	$(CC) -Ttext=0x1000 $(CFLAGS) ./$^ -o ./$@ $(LDFLAGS)

%.bin:%.elf
	$(COPY) -O binary ./$^ ./$@
	cp ./bios.bin ../image/

all: bios.bin virt.dtb

virt.dtb: virt.dts
	dtc -I dts -O dtb ./virt.dts > ./virt.dtb
	cp ./virt.dtb ../image/

clean:
	rm -rf ./*.elf ./*.bin ./*.dtb
