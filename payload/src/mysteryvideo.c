#include "mysteryvideo.h"
#include "../assets/assets.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <glob.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <errno.h>
#include <stdio.h>

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

static inline uint32_t get_pixel(uint32_t x, uint32_t y)
{
    return *(uint32_t *)(mysteryimage_buffer + (y * mysteryimage_stride) + (x * 4));
}

// Structs
struct card
{
    char *path;
    int fd;
    drmModeRes *resources;
};
struct renderer
{
    struct card *card;
    uint32_t connector_id;
    uint32_t crtc_id;

    drmModeModeInfo mode;
    uint32_t width;
    uint32_t height;

    uint32_t framebuffer_handle;
    uint64_t framebuffer_size;
    uint32_t *framebuffer;
    uint32_t framebuffer_id;
};

// Global vars
static int card_count = 0;
static struct card *cards[32] = {};
static int renderer_count = 0;
static struct renderer *renderers[256] = {};

void video_init()
{
    glob_t cards_glob = {};
    if (glob("/dev/dri/card*", 0, NULL, &cards_glob) != 0)
    {
        printf("Video error - Glob failed to enumerate /dev/dri/card*\n");
        fflush(stdout);
        goto cleanupGlob;
    }

    for (size_t i = 0; i < cards_glob.gl_pathc; i++)
    {
        struct card *card = NULL;

        for (int j = 0; j < card_count; j++)
        {
            if (strcmp(cards[j]->path, cards_glob.gl_pathv[i]) == 0)
            {
                goto cleanup;
            }
        }

        card = (struct card *)calloc(1, sizeof(struct card));

        card->path = calloc((strlen(cards_glob.gl_pathv[i]) + 1), sizeof(char));
        strcpy(card->path, cards_glob.gl_pathv[i]);

        card->fd = open(card->path, O_RDWR | O_CLOEXEC);
        if (card->fd < 0)
        {
            printf("Video error - open failed - %s\n", strerror(errno));
            fflush(stdout);
            goto cleanup;
        }

        if (drmSetMaster(card->fd) != 0)
        {
            printf("Video error - drmSetMaster failed - %s\n", strerror(errno));
            fflush(stdout);
            goto cleanup;
        }

        card->resources = drmModeGetResources(card->fd);
        if (card->resources == NULL)
        {
            printf("Video error - drmModeGetResources failed - %s\n", strerror(errno));
            fflush(stdout);
            goto cleanup;
        }

        if (card->resources->count_crtcs < card->resources->count_connectors)
        {
            printf("Video error - Card has %d connectors but only %d crtcs\n", card->resources->count_connectors, card->resources->count_crtcs);
            fflush(stdout);
            goto cleanup;
        }

        printf("Video info - Bound to card %s\n", card->path);
        fflush(stdout);

        cards[card_count] = card;
        card = NULL;
        card_count++;

    cleanup:
        if (card != NULL)
        {
            if (card->resources != NULL)
            {
                drmModeFreeResources(card->resources);
            }
            if (card->fd != 0)
            {
                close(card->fd);
            }
            if (card->path != NULL)
            {
                free(card->path);
            }
            free(card);
        }
    }

cleanupGlob:
    globfree(&cards_glob);

    for (int i = 0; i < card_count; i++)
    {
        for (int j = 0; j < cards[i]->resources->count_connectors; j++)
        {
            struct renderer *renderer = NULL;
            drmModeConnector *connector = NULL;

            for (int k = 0; k < renderer_count; k++)
            {
                if (strcmp(renderers[k]->card->path, cards[i]->path) == 0 && renderers[k]->connector_id == cards[i]->resources->connectors[j])
                {
                    goto cleanup2;
                }
            }

            renderer = (struct renderer *)calloc(1, sizeof(struct renderer));
            renderer->card = cards[i];
            renderer->connector_id = cards[i]->resources->connectors[j];
            renderer->crtc_id = cards[i]->resources->crtcs[j];

            connector = drmModeGetConnector(renderer->card->fd, renderer->connector_id);
            if (connector == NULL)
            {
                printf("Video error - drmModeGetConnector failed - %s\n", strerror(errno));
                fflush(stdout);
                goto cleanup2;
            }
            if (connector->connection != DRM_MODE_CONNECTED)
            {
                goto cleanup2;
            }

            /*
            uint32_t bound_crtc = 0;
            if (connector->encoder_id != 0)
            {
                drmModeEncoder *encoder = drmModeGetEncoder(renderer->card->fd, connector->encoder_id);
                if (encoder == NULL)
                {
                    printf("Video error - drmModeGetConnector failed - %s\n", strerror(errno));
                    fflush(stdout);
                    goto cleanup2;
                }
                bound_crtc = encoder->crtc_id;
                drmModeFreeEncoder(encoder);
            }
            */

            if (connector->count_modes <= 0)
            {
                printf("Video error - drmModeConnector has no modes\n");
                fflush(stdout);
                goto cleanup2;
            }
            renderer->mode = connector->modes[0];
            for (int j = 1; j < connector->count_modes; j++)
            {
                if (connector->modes[j].vdisplay * connector->modes[j].hdisplay > renderer->mode.vdisplay * renderer->mode.hdisplay)
                {
                    renderer->mode = connector->modes[j];
                }
                else if (connector->modes[j].vdisplay * connector->modes[j].hdisplay == renderer->mode.vdisplay * renderer->mode.hdisplay && connector->modes[j].vrefresh > renderer->mode.vrefresh)
                {
                    renderer->mode = connector->modes[j];
                }
            }
            renderer->width = renderer->mode.hdisplay;
            renderer->height = renderer->mode.vdisplay;

            struct drm_mode_create_dumb create_dumb = {};
            create_dumb.width = renderer->width;
            create_dumb.height = renderer->height;
            create_dumb.bpp = 32;
            create_dumb.flags = 0;
            if (drmIoctl(renderer->card->fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb) != 0)
            {
                printf("Video error - DRM_IOCTL_MODE_CREATE_DUMB failed - %s\n", strerror(errno));
                fflush(stdout);
                goto cleanup2;
            }
            renderer->framebuffer_handle = create_dumb.handle;
            renderer->framebuffer_size = create_dumb.size;

            struct drm_mode_map_dumb map_dumb = {};
            map_dumb.handle = renderer->framebuffer_handle;
            if (drmIoctl(renderer->card->fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) != 0)
            {
                printf("Video error - DRM_IOCTL_MODE_MAP_DUMB failed - %s\n", strerror(errno));
                fflush(stdout);
                goto cleanup2;
            }

            renderer->framebuffer = (uint32_t *)mmap(0, renderer->framebuffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, renderer->card->fd, map_dumb.offset);
            if (renderer->framebuffer == MAP_FAILED)
            {
                printf("Video error - mmap failed - %s\n", strerror(errno));
                fflush(stdout);
                goto cleanup2;
            }
            memset((void *)renderer->framebuffer, 0, renderer->framebuffer_size);

            if (drmModeAddFB(renderer->card->fd, renderer->width, renderer->height, 24, create_dumb.bpp, create_dumb.pitch, renderer->framebuffer_handle, &renderer->framebuffer_id) != 0)
            {
                printf("Video error - drmModeAddFB failed - %s\n", strerror(errno));
                fflush(stdout);
                goto cleanup2;
            }

            for (uint32_t x = 0; x < renderer->width; x++)
            {
                for (uint32_t y = 0; y < renderer->height; y++)
                {
                    uint32_t scaledX = (x * mysteryimage_width) / renderer->width;
                    uint32_t scaledY = (y * mysteryimage_height) / renderer->height;
                    uint32_t mysteryPixel = get_pixel(scaledX, scaledY);
                    renderer->framebuffer[(y * renderer->width) + x] = mysteryPixel;
                }
            }

            if (drmModeSetCrtc(renderer->card->fd, renderer->crtc_id, renderer->framebuffer_id, 0, 0, &renderer->connector_id, 1, &renderer->mode) != 0)
            {
                printf("Video error - drmModeSetCrtc failed - %s\n", strerror(errno));
                fflush(stdout);
            }

            printf("Video info - Bound to monitor %s\n", renderer->mode.name);
            fflush(stdout);

            renderers[renderer_count] = renderer;
            renderer = NULL;
            renderer_count++;

        cleanup2:
            if (connector != NULL)
            {
                drmModeFreeConnector(connector);
            }
        }
    }
}
void video_update()
{

}