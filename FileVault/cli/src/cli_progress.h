/*
 * FileVault v2.0 — CLI Progress Display
 * cli_progress.h — ANSI-based progress bar with speed and ETA
 */

#ifndef CLI_PROGRESS_H
#define CLI_PROGRESS_H

#include <stdint.h>

/*
 * Progress bar state — tracks timing for speed/ETA calculations.
 */
typedef struct {
    uint64_t total_bytes;
    double   start_time;
    int      bar_width;
    int      last_percent;
} CliProgress;

/*
 * Initialize progress tracking.
 */
void cli_progress_init(CliProgress *prog, uint64_t total_bytes);

/*
 * Update the progress bar display (called from FvProgressFn callback).
 * Uses \r to overwrite the current line.
 */
void cli_progress_update(uint64_t bytes_done, uint64_t bytes_total,
                         void *user_data);

/*
 * Finalize the progress display (print newline).
 */
void cli_progress_finish(CliProgress *prog);

/*
 * Format a byte count as human-readable (e.g., "4.2 MB").
 * buf must be at least 32 bytes.
 */
void cli_format_size(uint64_t bytes, char *buf, size_t buf_size);

#endif /* CLI_PROGRESS_H */
