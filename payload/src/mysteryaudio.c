#include "mysteryaudio.h"
#include "../build/assets_h/mysterysong_raw.h"
#include <alloca.h> // Required by asoundlib.h on musl
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include <stdbool.h>
#include <string.h>

static uint32_t get_sample_rate()
{
    return *(uint32_t *)(mysterysong_raw_buffer + 0);
}
static uint32_t get_channel_count()
{
    return *(uint32_t *)(mysterysong_raw_buffer + 4);
}
static uint32_t get_sample_count()
{
    return *(uint32_t *)(mysterysong_raw_buffer + 12);
}
static uint32_t *get_sample_ptr(uint32_t index)
{
    return (uint32_t *)(mysterysong_raw_buffer + 12 + (index * 4));
}

// Structs
struct device_context
{
    int card_id;
    int device_id;
    snd_pcm_t *handle;
    uint32_t position;
};

// Basic control flow
static int device_count = 0;
static struct device_context *devices[256];
void audio_init()
{
    int error_code = 0;
    int card_id = -1;
    while (true)
    {
        char *card_name = NULL;
        char *card_human_name = NULL;
        snd_mixer_t *mixer = NULL;
        snd_ctl_t *ctl = NULL;
        if ((error_code = snd_card_next(&card_id)) != 0)
        {
            printf("Audio error - snd_card_next failed - %s\n", snd_strerror(error_code));
            fflush(stdout);
            goto cleanup_card;
        }
        if (card_id < 0)
        {
            break;
        }

        card_name = (char *)malloc(snprintf(NULL, 0, "hw:%d", card_id) + 1);
        sprintf(card_name, "hw:%d", card_id);
        if ((error_code = snd_card_get_name(card_id, &card_human_name)) != 0)
        {
            const char unknown_card_name[] = "Unnamed sound card";
            card_human_name = malloc(sizeof(unknown_card_name));
            memcpy(card_human_name, unknown_card_name, sizeof(unknown_card_name));
        }
        printf("Audio info - Found sound card %s at %s\n", card_human_name, card_name);
        fflush(stdout);

        if ((error_code = snd_mixer_open(&mixer, 0)) != 0)
        {
            printf("Audio error - snd_mixer_open failed - %s\n", snd_strerror(error_code));
            fflush(stdout);
            goto cleanup_card;
        }
        if ((error_code = snd_mixer_attach(mixer, card_name)) != 0)
        {
            printf("Audio error - snd_mixer_attach failed - %s\n", snd_strerror(error_code));
            fflush(stdout);
            goto cleanup_card;
        }
        if ((error_code = snd_mixer_selem_register(mixer, NULL, NULL)) != 0)
        {
            printf("Audio error - snd_mixer_selem_register failed - %s\n", snd_strerror(error_code));
            fflush(stdout);
            goto cleanup_card;
        }
        if ((error_code = snd_mixer_load(mixer)) != 0)
        {
            printf("Audio error - snd_mixer_load failed - %s\n", snd_strerror(error_code));
            fflush(stdout);
            goto cleanup_card;
        }
        // elem is a pointer to mixer data and doesn't need to be freed. It's freed along with the mixer.
        snd_mixer_elem_t *elem = NULL;
        for (elem = snd_mixer_first_elem(mixer); elem; elem = snd_mixer_elem_next(elem))
        {
            const char *elem_name = snd_mixer_selem_get_name(elem);
            int elem_index = snd_mixer_selem_get_index(elem);
            if (snd_mixer_selem_has_playback_switch(elem))
            {
                printf("Audio Info - %s has control %s,%d with mute toggle. Unmuting...\n", card_name, elem_name, elem_index);
                if ((error_code = snd_mixer_selem_set_playback_switch_all(elem, 1)) != 0)
                {
                    printf("Audio error - snd_mixer_selem_set_playback_switch_all failed - %s\n", snd_strerror(error_code));
                    fflush(stdout);
                }
            }
            if (snd_mixer_selem_has_playback_volume(elem))
            {
                printf("Audio Info - %s has control %s,%d with volume. Setting to max...\n", card_name, elem_name, elem_index);
                long min_volume, max_volume;
                if ((error_code = snd_mixer_selem_get_playback_volume_range(elem, &min_volume, &max_volume)) != 0)
                {
                    printf("Audio error - snd_mixer_selem_get_playback_volume_range failed - %s\n", snd_strerror(error_code));
                    fflush(stdout);
                }
                else if ((error_code = snd_mixer_selem_set_playback_volume_all(elem, max_volume)) != 0)
                {
                    printf("Audio error - snd_mixer_selem_set_playback_volume_all failed - %s\n", snd_strerror(error_code));
                    fflush(stdout);
                }
            }
        }

        if ((error_code = snd_ctl_open(&ctl, card_name, 0)) != 0)
        {
            printf("Audio error - snd_ctl_open failed - %s\n", snd_strerror(error_code));
            fflush(stdout);
            goto cleanup_card;
        }
        int device_id = -1;
        while (true)
        {
            struct device_context *device = NULL;
            if ((error_code = snd_ctl_pcm_next_device(ctl, &device_id)) != 0)
            {
                printf("Audio error - snd_ctl_pcm_next_device failed - %s\n", snd_strerror(error_code));
                fflush(stdout);
                goto cleanup_device;
            }
            if (device_id < 0)
            {
                break;
            }
            device = (struct device_context *)calloc(1, sizeof(struct device_context));
            device->card_id = card_id;
            device->device_id = device_id;
            char device_name[256];
            sprintf(device_name, "hw:%d,%d", device->card_id, device->device_id);
            printf("Audio info - Found device %s.\n", device_name);
            fflush(stdout);
            if ((error_code = snd_pcm_open(&device->handle, device_name, SND_PCM_STREAM_PLAYBACK, 0)) != 0)
            {
                printf("Audio error - snd_pcm_open failed - %s\n", snd_strerror(error_code));
                fflush(stdout);
                goto cleanup_device;
            }
            snd_pcm_hw_params_t *params;
            snd_pcm_hw_params_alloca(&params);
            snd_pcm_hw_params_any(device->handle, params);
            if ((error_code = snd_pcm_hw_params_set_access(device->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) != 0)
            {
                printf("Audio error - snd_pcm_hw_params_set_access failed for SND_PCM_ACCESS_RW_INTERLEAVED - %s\n", snd_strerror(error_code));
                fflush(stdout);
                goto cleanup_device;
            }
            if ((error_code = snd_pcm_hw_params_set_format(device->handle, params, SND_PCM_FORMAT_S16_LE)) != 0)
            {
                printf("Audio error - snd_pcm_hw_params_set_format failed for SND_PCM_FORMAT_S32_LE - %s\n", snd_strerror(error_code));
                fflush(stdout);
                goto cleanup_device;
            }
            if ((error_code = snd_pcm_hw_params_set_channels(device->handle, params, get_channel_count())) != 0)
            {
                printf("Audio error - snd_pcm_hw_params_set_channels failed - %s\n", snd_strerror(error_code));
                fflush(stdout);
                goto cleanup_device;
            }
            int dir = 0;
            unsigned int samplerate = get_sample_rate();
            if ((error_code = snd_pcm_hw_params_set_rate_near(device->handle, params, &samplerate, &dir)) != 0 || samplerate != get_sample_rate())
            {
                printf("Audio error - snd_pcm_hw_params_set_rate_near failed - %s\n", snd_strerror(error_code));
                fflush(stdout);
                goto cleanup_device;
            }
            if ((error_code = snd_pcm_hw_params(device->handle, params)) != 0)
            {
                printf("Audio error - snd_pcm_hw_params failed - %s\n", snd_strerror(error_code));
                fflush(stdout);
                goto cleanup_device;
            }
            if ((error_code = snd_pcm_prepare(device->handle)) != 0)
            {
                printf("Audio error - snd_pcm_prepare failed - %s\n", snd_strerror(error_code));
                fflush(stdout);
                goto cleanup_device;
            }
            devices[device_count] = device;
            device_count++;
            device = NULL;

        cleanup_device:
            if (device != NULL)
            {
                if (device->handle != NULL)
                {
                    snd_pcm_close(device->handle);
                }
                free(device);
            }
        }

    cleanup_card:
        if (card_name != NULL)
        {
            free(card_name);
        }
        if (card_human_name != NULL)
        {
            free(card_human_name);
        }
        if (mixer != NULL)
        {
            snd_mixer_close(mixer);
        }
        if (ctl != NULL)
        {
            snd_ctl_close(ctl);
        }
    }
}
void audio_update()
{
    for (int i = 0; i < device_count; i++)
    {
        struct device_context *device = devices[i];

        uint32_t frames_to_write = get_sample_count() - device->position;
        if (frames_to_write > 44100)
        {
            frames_to_write = 44100;
        }

        int frames_written = snd_pcm_writei(device->handle, get_sample_ptr(device->position), frames_to_write);
        if (frames_written < 0)
        {
            printf("Audio error - snd_pcm_writei failed - %s - Falling back to snd_pcm_recover\n", strerror(errno));
            fflush(stdout);
            if (snd_pcm_recover(device->handle, frames_written, 0) != 0)
            {
                printf("Audio error - snd_pcm_recover failed - %s\n", strerror(errno));
                fflush(stdout);
            }
        }
        else
        {
            device->position = (device->position + frames_written) % get_sample_count();
        }
    }
}
void audio_cleanup()
{
}