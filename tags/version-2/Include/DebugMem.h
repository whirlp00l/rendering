// NOTE: MAKE sure to include this in all .cpp files. ALL OF THEM.

// This is really cool code. What it does is enable visual studio
// to detect memory leaks from malloc and new calls and tell you
// where the initial call site of the leak was!!!

// This is not enabled in release builds. Only in debug mode.

#ifdef _DEBUG
    #include <crtdbg.h>
    #define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
    #define new DEBUG_NEW
#else
    #define DEBUG_NEW new
#endif