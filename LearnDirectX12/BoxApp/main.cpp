#include "BoxApp.h"
#include <crtdbg.h>

int main() {
#if defined(_DEBUG) || defined(DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
}