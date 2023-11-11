#include <unistd.h>

int main() {
	long l = 323445;
	int i = 45392;
	short s = 1233;
	char c = 34;

	int i_overflow = 9999999999; 
	
	l = i;
	i = l;

	c = l;
	l = c;

	return 0;
}
