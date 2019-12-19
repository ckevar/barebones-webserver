#include "WebServer.h"

#include <cstdio>

int main(int argc, char const *argv[])
{
	WebServer webServer("0.0.0.0", 8080);
	if (webServer.init() != 0)
		return -1;

	webServer.run();

	getchar();
	// system("pause");
	return 0;
}