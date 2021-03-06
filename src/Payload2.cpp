unsigned int serial[] = { 0x31, 0x3e, 0x3d, 0x26, 0x31 };

int CheckByte(unsigned char *ptr) {
	int i, j = 0;
	int hash = 0xABCD;

	for (i = 0; ptr[i]; i++) {
		hash += ptr[i] ^ serial[j];

		j = (j == 4) ? 0 : j + 1;
	}

	return hash;
}

extern "C" unsigned char bufferByte[] = {
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 0
};

extern "C" int PayloadByte() {
	int ret;

	ret = CheckByte(bufferByte);
	if (ret == 0xad6d) {
		return 1;
	}

	return 0;
}