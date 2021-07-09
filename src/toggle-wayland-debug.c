#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static void print_usage(void) {
    fprintf(stderr, "Usage: toggle_debug [--enable / --disable]\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        print_usage();
        return 2;
    }

    int enable;
    if (strcmp(argv[1], "--enable") == 0) {
       enable = 1;
    } else if (strcmp(argv[1], "--disable") == 0) {
       enable = 0;
    } else {
        print_usage();
        return 2;
    }

    sem_t* semaphore = sem_open("/wayland-debug", O_CREAT, 0600, 0);
    if (semaphore == SEM_FAILED) {
        int err = errno;
        fprintf(stderr,
                "Failed to open debug semaphore. "
                "Error %d: %s\n", err, strerror(err));
        return 1;
    }

    if (enable) {
        if (sem_post(semaphore) < 0) {
            int err = errno;
            fprintf(stderr,
                    "Failed to increment debug semaphore. "
                    "Error %d: %s\n", err, strerror(err));
            return 1;
        } else {
            printf("Debugging enabled; logging to /tmp/wayland-debug\n");
        }
    } else {
        if (sem_trywait(semaphore) < 0) {
            int err = errno;
            fprintf(stderr,
                    "Failed to decrement debug semaphore. "
                    "Error %d: %s\n", err, strerror(err));
            return 1;
        } else {
            printf("Debugging disabled\n");
        }
    }
    return 0;
}
