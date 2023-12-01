#include <sys/inotify.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>


int main() {
    int inotify_fd, wd;

    if ((inotify_fd = inotify_init()) < 0) {
        printf("inotify_init failed %d:%s\n", errno, strerror(errno));
        return;
    }

    if ((wd = inotify_add_watch(inotify_fd, "/proc/net/ip_mr_cache", IN_MODIFY)) < 0) {
        printf("inotify_add_watch failed %d:%s\n", errno, strerror(errno));
        return;
    }


    struct inotify_event* event;
    int length = 0, i = 0, nread = 0;
    char buf[BUFSIZ] = {0};
    length = read(inotify_fd, buf, sizeof(buf) - 1);

    while (length > 0) {
        event = (struct inotify_event*)&buf[nread];

        // only file close write
        if (event->len == 0 && event->mask & IN_MODIFY) {
            printf("CHANGE!!!!!\n");
        }

        nread = nread + sizeof(struct inotify_event) + event->len;
        length = length - sizeof(struct inotify_event) - event->len;
    }

    return 0;
}
