#define _GNU_SOURCE
#include "mystery.h"
#include "mysteryvideo.h"
#include "mysteryaudio.h"
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

static void ignore_sig(int sig)
{
    (void)sig;
    return;
}
static void exit_sig(int sig)
{
    (void)sig;
    exit_requested = true;
}

volatile bool exit_requested = false;
int main(int argc, char **argv)
{
    bool no_audio = false;
    bool no_video = false;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--noaudio") == 0)
        {
            no_audio = true;
            printf("Mystery info - No audio\n");
            fflush(stdout);
        }
        if (strcmp(argv[i], "--novideo") == 0)
        {
            no_video = true;
            printf("Mystery info - No video\n");
            fflush(stdout);
        }
    }

    for (int i = 0; i < _NSIG; i++)
    {
        if (i == SIGINT)
        {
            signal(i, exit_sig);
        }
        else
        {
            signal(i, ignore_sig);
        }
    }

    if (!no_audio)
    {
        audio_init();
    }
    if (!no_video)
    {
        video_init();
    }

    unsigned long long time_last_frame = 0;

    while (!exit_requested)
    {
        if (!no_audio)
        {
            audio_update();
        }
        if (!no_video)
        {
            video_update();
        }

        struct timespec timespec_now;
        clock_gettime(CLOCK_MONOTONIC_RAW, &timespec_now);
        unsigned long long time_now = ((unsigned long long)timespec_now.tv_sec * 1000000000ULL) + (unsigned long long)timespec_now.tv_nsec;
        unsigned long long delta_time = time_now - time_last_frame;
        double fps = 1000000000.0 / (double)delta_time;
        printf("FPS: %lf TPF: %llu\n", fps, delta_time);
        fflush(stdout);
        time_last_frame = time_now;
    }

    printf("Mystery info - Quitting gracefully\n");
    fflush(stdout);

    if (!no_audio)
    {
        audio_cleanup();
    }
    if (!no_video)
    {
        video_cleanup();
    }

    return 0;
}