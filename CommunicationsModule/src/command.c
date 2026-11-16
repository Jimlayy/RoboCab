/*
 * command.c - Receives commands from pi_server.py over a Unix domain socket.
 *
 * Each datagram is exactly one command as text, e.g. "FORWARD",
 * "AUTO ON" or "START A3 B7". The command is parsed straight into a
 * shared command_data_t that the calculation thread reads with
 * command_data_get().
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "command.h"

#define CMD_SOCK_PATH "/tmp/robocab_cmd.sock"   // must match C_SOCK_PATH in pi_server.py

/* Shared between the command thread (writer) and the calculation thread (reader). */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static command_data_t latest;                    // zeroed: DRIVE_STOP, manual mode

void command_data_get(command_data_t *out)
{
    pthread_mutex_lock(&lock);
    *out = latest;
    latest.new_mission = 0;                      // a START is only reported once
    pthread_mutex_unlock(&lock);
}

/* Parses one command and updates the shared struct */
static void save_command(const char *cmd)
{
    char start[32], goal[32];

    pthread_mutex_lock(&lock);
    if      (strcmp(cmd, "FORWARD")  == 0) latest.drive = DRIVE_FORWARD;
    else if (strcmp(cmd, "BACK")     == 0) latest.drive = DRIVE_BACK;
    else if (strcmp(cmd, "LEFT")     == 0) latest.drive = DRIVE_LEFT;
    else if (strcmp(cmd, "RIGHT")    == 0) latest.drive = DRIVE_RIGHT;
    else if (strcmp(cmd, "STOP")     == 0) latest.drive = DRIVE_STOP;
    else if (strcmp(cmd, "AUTO ON")  == 0) latest.auto_mode = 1;
    else if (strcmp(cmd, "AUTO OFF") == 0) latest.auto_mode = 0;
    else if (sscanf(cmd, "START %31s %31s", start, goal) == 2) {
        strcpy(latest.start, start);
        strcpy(latest.goal, goal);
        latest.new_mission = 1;
    }
    else fprintf(stderr, "Unknown command: '%s'\n", cmd);
    pthread_mutex_unlock(&lock);
}

void *command_thread(void *arg)
{
    volatile sig_atomic_t *running = arg;

    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (fd < 0) { perror("command socket"); return NULL; }

    struct sockaddr_un addr = { .sun_family = AF_UNIX };
    strncpy(addr.sun_path, CMD_SOCK_PATH, sizeof addr.sun_path - 1);

    unlink(CMD_SOCK_PATH);                   // remove old socket file from last run
    if (bind(fd, (struct sockaddr *)&addr, sizeof addr) != 0) {
        perror("command bind");
        close(fd);
        return NULL;
    }

    struct pollfd pfd = { .fd = fd, .events = POLLIN };

    while (*running) {
        int r = poll(&pfd, 1, 500);          // timeout so we notice when running = 0
        if (r < 0) {
            if (errno == EINTR) continue;
            perror("command poll");
            break;
        }
        if (r == 0) continue;                // no command: just wait again

        char buf[64];
        ssize_t n = recv(fd, buf, sizeof buf - 1, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("command recv");
            break;
        }
        buf[n] = '\0';
        save_command(buf);
    }

    close(fd);
    unlink(CMD_SOCK_PATH);
    return NULL;
}
