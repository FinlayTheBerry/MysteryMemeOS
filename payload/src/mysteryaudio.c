#define _GNU_SOURCE
#include "mysteryaudio.h"
#include "../assets/assets.h"
#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <sys/mman.h>

#define check(condition, function, error)                                                      \
    do                                                                                         \
    {                                                                                          \
        if (!(condition))                                                                      \
        {                                                                                      \
            fprintf(stderr, "error - %s:%d - %s - %s\n", __FILE__, __LINE__, function, error); \
            fflush(stderr);                                                                    \
            _exit(1);                                                                          \
        }                                                                                      \
    } while (0);

static void bootstrap_alsa()
{
    const char mini_alsa_conf[] = "ctl.hw { @args [ CARD ] @args.CARD { type integer } type hw card $CARD } pcm.hw { @args [ CARD DEV ] @args.CARD { type integer } @args.DEV { type integer } type hw card $CARD device $DEV }";
    const size_t mini_alsa_conf_len = sizeof(mini_alsa_conf) - 1;

    int fd = memfd_create("mini_alsa_conf", 0);
    check(fd >= 0, "memfd_create", strerror(errno));

    ssize_t written = write(fd, mini_alsa_conf, mini_alsa_conf_len);
    check(written == (ssize_t)mini_alsa_conf_len, "write", strerror(errno));

    char fd_path[25] = {0}; // Just long enough for "/proc/self/fd/4294967296\0"
    snprintf(fd_path, sizeof(fd_path), "/proc/self/fd/%d", fd);

    int status = setenv("ALSA_CONFIG_PATH", fd_path, 1);
    check(status == 0, "setenv", strerror(errno));
}

static uint32_t *get_sample_ptr(uint32_t index)
{
    return (uint32_t *)(mysterysong_buffer + (index * mysterysong_bytes_per_sample * mysterysong_channel_count));
}

struct device_context
{
    snd_pcm_t *handle;
    uint32_t position;
};

static int device_count = 0;
static struct device_context devices[64] = {0};
void audio_init()
{
    bootstrap_alsa();

    int status = 0;
    int card_id = -1;
    while (true)
    {
        status = snd_card_next(&card_id);
        check(status == 0, "snd_card_next", snd_strerror(status));
        if (card_id < 0)
        {
            break;
        }

        char card_name[14] = {0}; // Just long enough for "hw:4294967296\0"
        sprintf(card_name, "hw:%d", card_id);

        snd_mixer_t *mixer = NULL;
        status = snd_mixer_open(&mixer, 0);
        check(status == 0, "snd_mixer_open", snd_strerror(status));

        status = snd_mixer_attach(mixer, card_name);
        check(status == 0, "snd_mixer_attach", snd_strerror(status));

        status = snd_mixer_selem_register(mixer, NULL, NULL);
        check(status == 0, "snd_mixer_selem_register", snd_strerror(status));

        status = snd_mixer_load(mixer);
        check(status == 0, "snd_mixer_load", snd_strerror(status));

        // elem is a pointer to mixer data and doesn't need to be freed. It's freed along with the mixer.
        for (snd_mixer_elem_t *elem = snd_mixer_first_elem(mixer); elem != NULL; elem = snd_mixer_elem_next(elem))
        {
            if (snd_mixer_selem_has_playback_switch(elem))
            {
                status = snd_mixer_selem_set_playback_switch_all(elem, 1);
                check(status == 0, "snd_mixer_selem_set_playback_switch_all", snd_strerror(status));
            }
            if (snd_mixer_selem_has_playback_volume(elem))
            {
                long min_volume = 0;
                long max_volume = 0;
                status = snd_mixer_selem_get_playback_volume_range(elem, &min_volume, &max_volume);
                check(status == 0, "snd_mixer_selem_get_playback_volume_range", snd_strerror(status));

                status = snd_mixer_selem_set_playback_volume_all(elem, max_volume);
                check(status == 0, "snd_mixer_selem_set_playback_volume_all", snd_strerror(status));
            }
        }

        status = snd_mixer_close(mixer);
        check(status == 0, "snd_mixer_close", snd_strerror(status));
        mixer = NULL;

        snd_ctl_t *ctl = NULL;
        status = snd_ctl_open(&ctl, card_name, 0);
        check(status == 0, "snd_ctl_open", snd_strerror(status));

        int device_id = -1;
        while (true)
        {
            status = snd_ctl_pcm_next_device(ctl, &device_id);
            check(status == 0, "snd_ctl_pcm_next_device", snd_strerror(status));
            if (device_id < 0)
            {
                break;
            }

            snd_pcm_info_t *info = NULL;
            snd_pcm_info_alloca(&info);
            snd_pcm_info_set_device(info, device_id);
            snd_pcm_info_set_subdevice(info, 0);
            snd_pcm_info_set_stream(info, SND_PCM_STREAM_PLAYBACK);
            status = snd_ctl_pcm_info(ctl, info);
            if (status == -ENOENT)
            {
                // Unable to get playback info so this device probably doesn't support playback.
                continue;
            }
            else if (status != 0)
            {
                check(false, "snd_ctl_pcm_info", snd_strerror(status));
            }

            char device_name[25]; // Just long enough for "hw:4294967296:4294967296\0"
            sprintf(device_name, "hw:%d,%d", card_id, device_id);

            snd_pcm_t *handle = NULL;
            status = snd_pcm_open(&handle, device_name, SND_PCM_STREAM_PLAYBACK, 0);
            check(status == 0, "snd_pcm_open", snd_strerror(status));

            snd_pcm_hw_params_t *params = NULL;
            snd_pcm_hw_params_alloca(&params);

            status = snd_pcm_hw_params_any(handle, params);
            check(status == 0, "snd_pcm_hw_params_any", snd_strerror(status));

            status = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
            check(status == 0, "snd_pcm_hw_params_set_access", snd_strerror(status));

            status = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
            check(status == 0, "snd_pcm_hw_params_set_format", snd_strerror(status));

            status = snd_pcm_hw_params_set_channels(handle, params, mysterysong_channel_count);
            check(status == 0, "snd_pcm_hw_params_set_channels", snd_strerror(status));

            int dir = 0;
            unsigned int framerate = mysterysong_frame_rate;
            status = snd_pcm_hw_params_set_rate_near(handle, params, &framerate, &dir);
            check(status == 0, "snd_pcm_hw_params_set_rate_near", snd_strerror(status));
            check(framerate == mysterysong_frame_rate, "snd_pcm_hw_params_set_rate_near", "Real framerate does not match target.");

            status = snd_pcm_hw_params(handle, params);
            check(status == 0, "snd_pcm_hw_params", snd_strerror(status));

            status = snd_pcm_prepare(handle);
            check(status == 0, "snd_pcm_prepare", snd_strerror(status));

            devices[device_count].handle = handle;
            devices[device_count].position = 0;
            device_count++;
            check(device_count < (int)sizeof(devices) / (int)sizeof(struct device_context), "device_count++", "Device count exceeds maximum.");
        }

        status = snd_ctl_close(ctl);
        check(status == 0, "snd_ctl_close", snd_strerror(status));
        ctl = NULL;
    }
}
void audio_update()
{
    int status = 0;
    for (int i = 0; i < device_count; i++)
    {
        struct device_context *device = &devices[i];
        int free_frames = snd_pcm_avail_update(device->handle);
        if (free_frames < 0)
        {
            status = snd_pcm_recover(device->handle, free_frames, 0);
            check(status == 0, "snd_pcm_recover", snd_strerror(status));
            continue;
        }
        if (free_frames == 0)
        {
            continue;
        }
        int frames_to_write = free_frames;
        if (mysterysong_frame_count - device->position < (uint32_t)frames_to_write)
        {
            frames_to_write = mysterysong_frame_count - device->position;
        }
        int frames_written = snd_pcm_writei(device->handle, get_sample_ptr(device->position), frames_to_write);
        if (frames_written < 0)
        {
            status = snd_pcm_recover(device->handle, frames_written, 0);
            check(status == 0, "snd_pcm_recover", snd_strerror(status));
            continue;
        }
        device->position = (device->position + frames_written) % mysterysong_frame_count;
    }
}