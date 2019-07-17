#include <iostream>
#include <ctime>
#include "system.h"

using namespace std;

int main(int argc, char *argv[])
{
	srand(static_cast <unsigned>(time(NULL)));

	System system(&argc, argv);

	system.run();

	return 0;
}
