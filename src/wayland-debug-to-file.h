#ifndef WAYLAND_INIT_DEBUG_H
#define WAYLAND_INIT_DEBUG_H

#include <semaphore.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

struct wl_debug_to_file {
	sem_t* enabled_semaphore;
	FILE* log_file;
	int is_enabled;
};

/**
 * Set up to log protocol traffic to a file under /tmp/wayland-debug.
 *
 * None of the functions that operate on wl_debug_to_file are thread-safe.
 * Do not call them concurrently.
 */
void
wl_init_debug_to_file(struct wl_debug_to_file* data);

/**
 * Cease logging protocol traffic (counterpart to wl_init_debug_to_file()).
 *
 * None of the functions that operate on wl_debug_to_file are thread-safe.
 * Do not call them concurrently.
 */
void
wl_cleanup_debug_to_file(struct wl_debug_to_file* data);

/*
 * Returns a file handle for logging protocol traffic, or NULL if logging is
 * disabled.
 *
 * None of the functions that operate on wl_debug_to_file are thread-safe.
 * Do not call them concurrently.
 */
FILE*
wl_debug_log(struct wl_debug_to_file* data, int is_server);

#endif
