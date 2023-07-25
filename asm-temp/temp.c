#include <unistd.h>

int babi() {
	int i = 42;
	write(1, &i, 1);
	return i;
}

int main() {
	int j = babi();
	return 0;
}
