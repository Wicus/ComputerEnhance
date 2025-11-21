#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

typedef unsigned long long u64;

u64 ReadTimestampCounter(void);
u64 EstimateCpuTimerFreq(void);

#define DEFAULT_SAMPLES 5
#define SLEEP_MS 10

static int ParseSamples(int argc, char **argv)
{
    if (argc < 2)
    {
        return DEFAULT_SAMPLES;
    }

    int value = atoi(argv[1]);
    if (value <= 0)
    {
        return -1;
    }

    return value;
}

static void RunSamples(int samples)
{
    printf("samples (%ums sleep)\n", SLEEP_MS);

    for (int i = 0; i < samples; ++i)
    {
        u64 start = ReadTimestampCounter();
        Sleep(SLEEP_MS);
        u64 end = ReadTimestampCounter();
        u64 elapsed = end - start;

        printf("sample %d: %llu ticks\n", i + 1, elapsed);
    }
}

int main(int argc, char **argv)
{
    int samples = ParseSamples(argc, argv);
    if (samples <= 0)
    {
        fprintf(stderr, "invalid sample count\n");
        return 1;
    }

    u64 freq = EstimateCpuTimerFreq();
    printf("rdtsc harness\n");
    printf("estimated freq: %llu Hz\n\n", freq);

    RunSamples(samples);
    return 0;
}
