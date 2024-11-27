#include <unistd.h>

int main() {
	char nl = 10;

	int i = 5;
	char c = 2;
	long l = 48;
	short s = 8;
	
	i = c+4;
	i = i+l;
	write(1, &i, (1+c-2));
	write(1, &nl, 1);
}
