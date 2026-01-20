/*
 * device_data_logger.c
 *
 * What this program does:
 * - Generates a given number of fake sensor readings (temperature, humidity, timestamp, sensor ID)
 * - Writes the records to a binary file
 * - Optionally reads and displays a specific record at a given index
 *
 * Build:
 *   gcc -O2 -Wall -Wextra -std=c11 device_data_logger.c -o device_logger
 *
 * Run:
 *   ./device_logger <count> <file>
 *   ./device_logger <count> <file> --show <index>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

// Struct to hold sensor data
typedef struct {
    uint32_t sensor_id;
    float temperature_celsius;
    float humidity_percent;
    time_t timestamp;
} SensorRecord;

// Exit program with an error message
void exit_on_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// Generate a random float between min and max
float random_float(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

// Fill a SensorRecord with random data
void create_random_record(SensorRecord *rec, uint32_t id) {
    rec->sensor_id = id;
    rec->temperature_celsius = random_float(-5.0f, 55.0f);
    rec->humidity_percent = random_float(10.0f, 100.0f);
    rec->timestamp = time(NULL);
}

// Write all records to a binary file
void write_records_to_file(const char *path, SensorRecord *records, size_t count) {
    int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0)
        exit_on_error("Failed to open file for writing");

    ssize_t written = write(fd, records, count * sizeof(SensorRecord));
    if (written < 0)
        exit_on_error("Failed to write to file");

    close(fd);
}

// Read and print a specific record by index
void read_record_by_index(const char *path, size_t index) {
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        exit_on_error("Failed to open file for reading");

    off_t offset = index * sizeof(SensorRecord);
    if (lseek(fd, offset, SEEK_SET) == (off_t)-1)
        exit_on_error("Failed to seek to record position");

    SensorRecord rec;
    ssize_t read_bytes = read(fd, &rec, sizeof(SensorRecord));
    if (read_bytes != sizeof(SensorRecord)) {
        fprintf(stderr, "Invalid index or incomplete record.\n");
        exit(EXIT_FAILURE);
    }

    printf("Sensor #%u\n", rec.sensor_id);
    printf("  Temperature: %.2f°C\n", rec.temperature_celsius);
    printf("  Humidity:    %.2f%%\n", rec.humidity_percent);
    printf("  Timestamp:   %s", ctime(&rec.timestamp));

    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3 && argc != 5) {
        fprintf(stderr, "Usage: %s <count> <file> [--show <index>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t count = (size_t)strtoul(argv[1], NULL, 10);
    if (count == 0) {
        fprintf(stderr, "Count must be a positive number.\n");
        return EXIT_FAILURE;
    }

    const char *filepath = argv[2];

    srand((unsigned int)time(NULL)); // Seed RNG

    SensorRecord *data = malloc(sizeof(SensorRecord) * count);
    if (!data) exit_on_error("Memory allocation failed");

    for (size_t i = 0; i < count; ++i) {
        create_random_record(&data[i], (uint32_t)(i + 1000)); // sensor_id = 1000 + i
    }

    write_records_to_file(filepath, data, count);

    if (argc == 5 && strcmp(argv[3], "--show") == 0) {
        size_t index = (size_t)strtoul(argv[4], NULL, 10);
        read_record_by_index(filepath, index);
    }

    free(data);
    return EXIT_SUCCESS;
}

