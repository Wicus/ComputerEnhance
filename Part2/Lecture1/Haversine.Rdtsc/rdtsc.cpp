#ifdef _WIN32
#  include <intrin.h>
#  define EXPORT __declspec(dllexport)
extern "C" EXPORT unsigned long long ReadTSC()
{
    return __rdtsc();
}
#else
#  include <stdint.h>
#  if defined(__i386__) || defined(__x86_64__)
static inline uint64_t read_tsc()
{
    unsigned int lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}
#  else
#    error "rdtsc not supported on this architecture"
#  endif
extern "C" __attribute__((visibility("default"))) unsigned long long ReadTSC()
{
    return read_tsc();
}
#endif
