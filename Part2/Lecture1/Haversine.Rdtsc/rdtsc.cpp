#include <intrin.h>

extern "C" __declspec(dllexport) unsigned long long ReadTSC()
{
    return __rdtsc();
}
