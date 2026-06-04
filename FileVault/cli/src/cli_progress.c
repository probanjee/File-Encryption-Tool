/*
 * FileVault v2.0 — cli_progress.c
 * ANSI progress bar with speed and ETA display.
 */

#include "cli_progress.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <time.h>
#  include <sys/ioctl.h>
#  include <unistd.h>
#endif

static double progress_get_time(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, count;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&count);
    return (double)count.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
#endif
}

static int get_terminal_width(void) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 80;
#else
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        return ws.ws_col;
    }
    return 80;
#endif
}

void cli_format_size(uint64_t bytes, char *buf, size_t buf_size) {
    if (bytes < 1024ULL) {
        snprintf(buf, buf_size, "%llu B", (unsigned long long)bytes);
    } else if (bytes < 1024ULL * 1024) {
        snprintf(buf, buf_size, "%.1f KB", (double)bytes / 1024.0);
    } else if (bytes < 1024ULL * 1024 * 1024) {
        snprintf(buf, buf_size, "%.1f MB", (double)bytes / (1024.0 * 1024.0));
    } else {
        snprintf(buf, buf_size, "%.2f GB",
                 (double)bytes / (1024.0 * 1024.0 * 1024.0));
    }
}

void cli_progress_init(CliProgress *prog, uint64_t total_bytes) {
    if (!prog) return;
    prog->total_bytes  = total_bytes;
    prog->start_time   = progress_get_time();
    prog->bar_width    = 30;
    prog->last_percent = -1;
}

void cli_progress_update(uint64_t bytes_done, uint64_t bytes_total,
                         void *user_data) {
    CliProgress *prog = (CliProgress *)user_data;
    if (!prog || bytes_total == 0) return;

    int percent = (int)((bytes_done * 100ULL) / bytes_total);

    /* Only update display when percentage changes (avoid flicker) */
    if (percent == prog->last_percent) return;
    prog->last_percent = percent;

    double elapsed = progress_get_time() - prog->start_time;

    /* Calculate speed */
    char speed_buf[32];
    if (elapsed > 0.1) {
        double speed = (double)bytes_done / elapsed;
        cli_format_size((uint64_t)speed, speed_buf, sizeof(speed_buf));
        /* Append "/s" */
        size_t len = strlen(speed_buf);
        if (len + 3 < sizeof(speed_buf)) {
            speed_buf[len] = '/';
            speed_buf[len + 1] = 's';
            speed_buf[len + 2] = '\0';
        }
    } else {
        snprintf(speed_buf, sizeof(speed_buf), "---");
    }

    /* Calculate ETA */
    char eta_buf[32];
    if (elapsed > 0.5 && bytes_done > 0 && bytes_done < bytes_total) {
        double rate = (double)bytes_done / elapsed;
        double remaining = (double)(bytes_total - bytes_done) / rate;
        if (remaining < 60.0) {
            snprintf(eta_buf, sizeof(eta_buf), "%.0fs", remaining);
        } else if (remaining < 3600.0) {
            snprintf(eta_buf, sizeof(eta_buf), "%.0fm%.0fs",
                     remaining / 60.0, ((int)remaining) % 60 * 1.0);
        } else {
            snprintf(eta_buf, sizeof(eta_buf), "%.0fh",
                     remaining / 3600.0);
        }
    } else if (bytes_done >= bytes_total) {
        snprintf(eta_buf, sizeof(eta_buf), "done");
    } else {
        snprintf(eta_buf, sizeof(eta_buf), "---");
    }

    /* Calculate bar width based on terminal */
    int term_width = get_terminal_width();
    int bar_width = term_width - 40; /* Leave room for percentage, speed, ETA */
    if (bar_width < 10) bar_width = 10;
    if (bar_width > 50) bar_width = 50;

    /* Build progress bar */
    int filled = (percent * bar_width) / 100;

    fprintf(stderr, "\r  ");

    /* Draw bar */
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) {
            fprintf(stderr, "\xe2\x96\x88"); /* █ full block */
        } else {
            fprintf(stderr, "\xe2\x96\x91"); /* ░ light shade */
        }
    }

    fprintf(stderr, " %3d%%  %s  ETA: %s  ", percent, speed_buf, eta_buf);
    fflush(stderr);
}

void cli_progress_finish(CliProgress *prog) {
    (void)prog;
    fprintf(stderr, "\n");
    fflush(stderr);
}
