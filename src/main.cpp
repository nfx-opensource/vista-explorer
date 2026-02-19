#include "Application.h"

#ifdef _WIN32
extern "C"
{
    __declspec( dllexport ) unsigned long NvOptimusEnablement = 1;
    __declspec( dllexport ) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#ifdef __linux__
#    include <cstdlib>
#endif

int main()
{
#ifdef __linux__
    setenv( "__NV_PRIME_RENDER_OFFLOAD", "1", 1 );
    setenv( "__GLX_VENDOR_LIBRARY_NAME", "nvidia", 1 );
#endif

    nfx::vista::Application app;
    app.initialize();

    app.run();

    return 0;
}
