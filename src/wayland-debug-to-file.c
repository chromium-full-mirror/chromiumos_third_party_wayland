#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "wayland-debug-to-file.h"

void
wl_init_debug_to_file(struct wl_debug_to_file* debug) {
	debug->is_enabled = 0;
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

void
wl_open_debug_file(struct wl_debug_to_file* debug, int is_server) {
	char debug_log_filename[255];
	static int counter = 0;

	if (debug == NULL
		|| debug->log_file != NULL
		|| debug->enabled_semaphore == SEM_FAILED) {
		return;
	}

	// Open a file with a probably-unique filename for writing.
	// Between the PID, an arbitrary pointer address, and
	// the static counter, filenames are unlikely to repeat.
	if (snprintf(debug_log_filename, sizeof(debug_log_filename),
				is_server
					? "/tmp/wayland-debug/server-pid%d-%p-%d.log"
					: "/tmp/wayland-debug/client-pid%d-%p-%d.log",
				getpid(), (void*)debug, counter) < 0) {
		perror("Failed to format debug log filename; debugging not enabled");
		return;
	}
	debug->log_file = fopen(debug_log_filename, "w");
	if (debug->log_file == NULL) {
		perror("Failed to open debug log; debugging not enabled");
		return;
	}
	counter++;
}

void
wl_close_debug_file(struct wl_debug_to_file* debug) {
	if (debug == NULL || debug->log_file == NULL) return;

	// Write a sentinel to the end of the file to mark completion.
	fprintf(debug->log_file, "+++LOG END+++\n");
	fclose(debug->log_file);
	debug->log_file = NULL;
}

FILE*
wl_debug_log(struct wl_debug_to_file* debug, int is_server) {
	int is_enabled;
	sem_t* sem = debug->enabled_semaphore;

	// The semaphore is "unlocked" (value set to 1) to enable debugging, and
	// "locked" (value set to 0, the default) to disable.
	if (sem == SEM_FAILED || sem_getvalue(sem, &is_enabled) < 0) {
		// On error, silently disable debugging.
		is_enabled = 0;
	}

	// Open/close the log file when debugging is enabled/disabled.
	if (is_enabled != debug->is_enabled) {
		if (is_enabled) {
			wl_open_debug_file(debug, is_server);
		} else {
			wl_close_debug_file(debug);
		}
	}
	debug->is_enabled = is_enabled;
	return is_enabled ? debug->log_file : NULL;
}
