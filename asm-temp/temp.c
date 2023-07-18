#include <unistd.h>

int main() {
	int i = 10;
	write(1, &i, 1);
	return 0;
}

