KDIR=qemu-rpi-kernel
ZIMAGE=$(KDIR)/kernel-qemu-4.4.34-jessie
ZIMAGE_GIT=https://github.com/dhruvvyas90/qemu-rpi-kernel.git

RASP_VERSION=raspbian-2017-03-03
RASP_NAME=2017-03-02-raspbian-jessie
OFFSET=70254592
RIMAGE_URL=http://downloads.raspberrypi.org/raspbian/images/$(RASP_VERSION)/$(RASP_NAME).zip


RIMAGE=$(RASP_NAME).img
ROFFSET=$(OFFSET)

TEMPDIR := $(shell mktemp -u)

QEMU_DISPLAY ?= sdl

QEMU_OPTS=-kernel $(ZIMAGE) \
	-device virtio-serial \
	-chardev pty,id=virtiocon0 -device virtconsole,chardev=virtiocon0 \
	-cpu arm1176 \
	-m 256 \
	-M versatilepb \
	-net nic,model=virtio,vlan=0 -net tap,ifname=tap0,vlan=0,script=no,downscript=no \
	-hda $(RIMAGE) \
	--append "root=/dev/sda2 rootfstype=ext4 rw" \
	--display $(QEMU_DISPLAY) -s

$(ZIMAGE):
	git clone https://github.com/dhruvvyas90/qemu-rpi-kernel.git $(KDIR)

$(RASP_NAME).zip:
	wget $(RIMAGE_URL)

$(RIMAGE): $(RASP_NAME).zip
	unzip $(RASP_NAME).zip

.modinst: $(ZIMAGE) $(RIMAGE)
	mkdir $(TEMPDIR)
	sudo mount -v -o offset=$(ROFFSET) -t ext4 $(RIMAGE) $(TEMPDIR)
	sudo umount $(TEMPDIR)
	rmdir $(TEMPDIR)

tap0:
	./create_net.sh $@

boot: .modinst tap0
	ARCH=arm ./qemu.sh $(QEMU_OPTS)

clean:
	rm -f .modinst

.PHONY: clean tap0


