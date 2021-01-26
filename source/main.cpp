#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace std;

// Entry point for the EndlessSky executable
int main(int argc, char *argv[])
{
	for(const char *const *it = argv + 1; *it; ++it)
	{
		string arg = *it;
		if(arg == "-h" || arg == "--help")
		{
			cout << "Help!" << endl;
			return 0;
		}
	}

	cout << "Running program." << endl;
}
