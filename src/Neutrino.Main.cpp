#include <cstdio>
#include <cstdint>

int Payload(int in) {
	if (in < 0) {
		return  100 - in;
	} else {
		return in * in - 50;
	}
}

struct JumpTable {
	uintptr_t orig; // original jump destination
	uintptr_t hook; // new destination
};

class Translator {
private :
public :
	uintptr_t nextBb;
};


int main() {
	printf("Neutrino\n");

	printf("Payload(%d) = %d\n", 10, Payload(10));
	printf("Payload(%d) = %d\n", -3, Payload(-3));

	return 0;
}