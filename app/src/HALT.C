#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
	char *s = "\n\aSystem is going to halt NOW!\a\n";
	
	write(fileno(stdin), s, strlen(s));
	sync();
	sleep(3);
	sync();
	sync();
	if (reboot('m','h')) {     /* reboot('m','e')=REBOOT ; reboot('m','h')=HALT */
		perror("halt");
		return (errno);
	}
}

