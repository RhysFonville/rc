#include <unistd.h>

char babi() {
	char i = 42;
	write(1, &i, 1);
	return i;
}

int main() {
	char j = babi();
	return 0;
}
