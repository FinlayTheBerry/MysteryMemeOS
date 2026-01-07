#define _GNU_SOURCE
#include "mystery.h"
#include "mysteryvideo.h"
#include "mysteryaudio.h"
#include <stdbool.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>

#define check(condition, function, error) do { if (!(condition)) { fprintf(stderr, "error - %s:%d - %s - %s\n", __FILE__, __LINE__, function, error); fflush(stderr); _exit(1); } } while (0);

static void exit_sig(int sig)
{
    (void)sig;
    _exit(0);
}

int main(int argc, char **argv)
{
    int status = 0;
    bool no_audio = false;
    bool no_video = false;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--noaudio") == 0)
        {
            no_audio = true;
            fprintf(stdout, "Mystery info - No audio\n");
            fflush(stdout);
        }
        if (strcmp(argv[i], "--novideo") == 0)
        {
            no_video = true;
            fprintf(stdout, "Mystery info - No video\n");
            fflush(stdout);
        }
    }

    signal(SIGINT, exit_sig);

    if (!no_audio)
    {
        audio_init();
    }
    if (!no_video)
    {
        video_init();
    }

    struct timespec timespec_now;
    status = clock_gettime(CLOCK_MONOTONIC_RAW, &timespec_now);
    check(status == 0, "clock_gettime", strerror(errno));
    uint64_t time_last_frame = ((uint64_t)timespec_now.tv_sec * (uint64_t)1000000000) + (uint64_t)timespec_now.tv_nsec;
    while (true)
    {
        if (!no_audio)
        {
            audio_update();
        }
        if (!no_video)
        {
            video_update();
        }

        status = clock_gettime(CLOCK_MONOTONIC_RAW, &timespec_now);
        check(status == 0, "clock_gettime", strerror(errno));
        uint64_t time_now = ((uint64_t)timespec_now.tv_sec * (uint64_t)1000000000) + (uint64_t)timespec_now.tv_nsec;

        uint64_t delta_time = time_now - time_last_frame;
        double fps = (double)1000000000.0 / (double)delta_time;
        printf("FPS: %lf TPF: %" PRIu64 "\r", fps, delta_time);
        fflush(stdout);
        time_last_frame = time_now;
    }
}