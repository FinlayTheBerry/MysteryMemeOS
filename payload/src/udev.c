#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#define NL_MAX_PAYLOAD 4096

int udev_main()
{
    char msg[NL_MAX_PAYLOAD];

    struct sockaddr_nl src_addr = {};
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 1; // Subscribe only to the kernel not udev or other daemons

    int nl_socket = socket(AF_NETLINK, (SOCK_DGRAM | SOCK_CLOEXEC), NETLINK_KOBJECT_UEVENT);
    if (nl_socket < 0)
    {
        printf("Udev error - socket NETLINK_KOBJECT_UEVENT failed - %s\n", strerror(errno));
        fflush(stdout);
        return 1;
    }

    if (bind(nl_socket, (struct sockaddr *)&src_addr, sizeof(src_addr)) != 0)
    {
        printf("Udev error - bind failed - %s\n", strerror(errno));
        fflush(stdout);
        return 1;
    }

    printf("Udev info - Bound and listening\n");
    fflush(stdout);
    while (true)
    {
        int len = recv(nl_socket, msg, sizeof(msg), MSG_DONTWAIT);
        if (len < 0 && errno == EAGAIN) {
            continue;
        }
        if (len < 0)
        {
            printf("Udev error - recv failed - %s\n", strerror(errno));
            fflush(stdout);
            return 1;
        }
        char* ptr = msg;
        while ((size_t)ptr - (size_t)msg < (size_t)len) {
            printf("%s\n", ptr);
            ptr += strlen(ptr) + 1;
        }
        printf("\n");
        fflush(stdout);
    }
    return 0;
}