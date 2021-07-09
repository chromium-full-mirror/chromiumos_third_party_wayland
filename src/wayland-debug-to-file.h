#ifndef WAYLAND_INIT_DEBUG_H
#define WAYLAND_INIT_DEBUG_H

#include <semaphore.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

struct wl_debug_to_file {
	sem_t* enabled_semaphore;
	FILE* log_file;
	int was_enabled;
};

/**
 * Set up to log protocol traffic to a file under /tmp/wayland-debug.
 */
void
wl_init_debug_to_file(int is_server, struct wl_debug_to_file* data);

void
wl_cleanup_debug_to_file(struct wl_debug_to_file* data);

/*
 * Returns true if we should write protocol traffic to files.
 */
bool
is_debug_to_file_enabled(struct wl_debug_to_file* data);

#endif
