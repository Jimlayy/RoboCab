#include <pthread.h>
#include "../include/sensor_data.h"

/* Shared between the port thread (writer) and the calculation thread (reader). */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static sensor_data_t latest;
static int has_data = 0;

void sensor_data_update(const sensor_data_t *s)
{
    pthread_mutex_lock(&lock);
    latest = *s;
    has_data = 1;
    pthread_mutex_unlock(&lock);
}

int sensor_data_get(sensor_data_t *out)
{
    int result = -1;

    pthread_mutex_lock(&lock);
    if (has_data) {
        *out = latest;
        result = 0;
    }
    pthread_mutex_unlock(&lock);
    return result;
}
