#ifndef COMMAND_H
#define COMMAND_H

/* Manual driving commands. DRIVE_STOP is 0, so a zeroed struct means "stand still". */
typedef enum {
    DRIVE_STOP = 0,
    DRIVE_FORWARD,
    DRIVE_BACK,
    DRIVE_LEFT,
    DRIVE_RIGHT
} drive_cmd_t;

/* Latest commands from the GUI. Each command type has its own field,
 * so a new command never overwrites a different one. */
typedef struct {
    drive_cmd_t drive;      // latest manual driving command
    int auto_mode;          // 1 = autonomous, 0 = manual
    char start[32];         // pickup
    char goal[32];          // drop-off
    int new_mission;        // 1 the first time it is read after a START
} command_data_t;

/* Thread that receives commands from pi_server.py on a Unix domain socket.
 * arg = pointer to the running flag (volatile sig_atomic_t). */
void *command_thread(void *arg);

/* Copies the latest commands into out and clears new_mission. */
void command_data_get(command_data_t *out);

#endif
