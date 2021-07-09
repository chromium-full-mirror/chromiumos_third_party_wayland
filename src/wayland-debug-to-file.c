#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "wayland-debug-to-file.h"

void
wl_init_debug_to_file(int is_server, struct wl_debug_to_file* debug) {
	char debug_log_filename[255];
	debug->was_enabled = 0;
	debug->enabled_semaphore = SEM_FAILED;
	debug->log_file = NULL;

	debug->enabled_semaphore = sem_open("/wayland-debug", O_CREAT, 0600, 0);
	if (debug->enabled_semaphore == SEM_FAILED) {
		perror("Failed to open debug semaphore; debugging disabled");
		return;
	}

	if (mkdir("/tmp/wayland-debug", 0700) < 0 && errno != EEXIST) {
		perror("mkdir /tmp/wayland-debug failed; debugging disabled");
		wl_cleanup_debug_to_file(debug);
		return;
	}
	if (snprintf(debug_log_filename, sizeof(debug_log_filename),
				 is_server
					 ? "/tmp/wayland-debug/server-pid%d-%p.log"
					 : "/tmp/wayland-debug/client-pid%d-%p.log",
				 getpid(), (void*)debug) < 0) {
		perror("Failed to format debug log filename; debugging disabled");
		wl_cleanup_debug_to_file(debug);
		return;
	}
	debug->log_file = fopen(debug_log_filename, "w");
	if (debug->log_file == NULL) {
		perror("Failed to open debug log; debugging disabled");
		wl_cleanup_debug_to_file(debug);
		return;
	}
}

void
wl_cleanup_debug_to_file(struct wl_debug_to_file* debug) {
	if (debug == NULL) return;

	if (debug->log_file != NULL) {
		fclose(debug->log_file);
		debug->log_file = NULL;
	}
	if (debug->enabled_semaphore != SEM_FAILED) {
		sem_close(debug->enabled_semaphore);
		debug->enabled_semaphore = SEM_FAILED;
	}
}

bool
is_debug_to_file_enabled(struct wl_debug_to_file* debug) {
	int is_enabled;
	sem_t* sem = debug->enabled_semaphore;
	if (sem == SEM_FAILED
			|| debug->log_file == NULL
			|| sem_getvalue(sem, &is_enabled) < 0) {
		// On error, silently disable debugging.
		is_enabled = 0;
	}

	// Flush the log when debugging is disabled.
	if (!is_enabled && debug->was_enabled) {
		if (debug->log_file != NULL && fflush(debug->log_file) != 0) {
			perror("libwayland: Debug log not flushed, may be incomplete");
		}
	}
	debug->was_enabled = is_enabled;

	// The semaphore is "unlocked" (value set to 1) to enable debugging, and
	// "locked" (value set to 0, the default) to disable.
	return is_enabled;
}
