/* 
 * File:   capture.c
 * Author: raslanove
 *
 * Created on October 3, 2021
 */

// See: https://www.lemoda.net/c/mmap-example/index.html

// For the size of the file,
#include <sys/stat.h>

// Contains the mmap calls,
#include <sys/mman.h> 

// For error printing,
#include <errno.h>
#include <string.h>
#include <stdarg.h>

// For open(),
#include <fcntl.h>
#include <stdio.h>

// For write(),
#include <unistd.h>

// For frame timing,
#include <time.h>

// For exit(),
#include <stdlib.h>

// Test condition, if true, prints the message,
static void check(int condition, const char * message, ...) {
    if (condition) {
        va_list args;
        va_start(args, message);
        vfprintf(stderr, message, args);
        va_end(args);
        fprintf(stderr, "\n");
        exit(EXIT_FAILURE);
    }
}

struct FileMemoryMapping {
    const char* memory;
    size_t size;
};

static struct FileMemoryMapping mapFileToMemory(const char* fileName) {

    int fileDescriptor;
    struct stat fileStat;
    struct FileMemoryMapping mapping;

    // Get file descriptor,
    fileDescriptor = open(fileName, O_RDONLY);
    check(fileDescriptor < 0, "open %s failed: %s", fileName, strerror(errno));

    // Get the size of the file,
    check(fstat(fileDescriptor, &fileStat) < 0, "stat %s failed: %s", fileName, strerror(errno));
    mapping.size = fileStat.st_size;

    // Memory-map the file,
    mapping.memory = mmap(0, mapping.size, PROT_READ, MAP_PRIVATE, fileDescriptor, 0);
    check(mapping.memory == MAP_FAILED, "mmap %s failed: %s", fileName, strerror(errno));
    
    return mapping;    
}

static double timeMillisSince(struct timespec* startTime) {
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);
    double timeNanos = 
        (currentTime.tv_sec  - startTime->tv_sec )*1E9 +
        (currentTime.tv_nsec - startTime->tv_nsec);
    return timeNanos / 1E6;
}

static void addToTime(struct timespec* outTime, double timeToAddMillis) {
    outTime->tv_nsec += timeToAddMillis * 1E6;
    if (outTime->tv_nsec >= 1E9) {
        outTime->tv_sec++;
        outTime->tv_nsec -= 1E9;
    }
}

static void captureAndOutputFrames(struct FileMemoryMapping mappedFrameBuffer, int32_t rate) {
    
    double frameDurationMillis = 1000.0 / rate;
    struct timespec frameStartTime;
        
    clock_gettime(CLOCK_REALTIME, &frameStartTime);    
    while (1) {
        
        // Write the frame,        
        write(1 /*stdout*/, mappedFrameBuffer.memory, mappedFrameBuffer.size);
        
        // Sleep till the end of the frame,
        double elapsedTimeMillis = timeMillisSince(&frameStartTime);
        if (elapsedTimeMillis < frameDurationMillis) usleep((unsigned int) (1000 * (frameDurationMillis - elapsedTimeMillis)));
        
        // Update next frame start time,
        addToTime(&frameStartTime, frameDurationMillis);
    }
}

int main(int argc, char* argv[]) {
    
    // Make sure we have a sufficient arguments,
    int rate;
    check(argc < 3, "Usage: ./capture.o <filename> <frame rate>\n");
    sscanf (argv[2], "%d", &rate);

    // Map file to memory,
    struct FileMemoryMapping mappedFrameBuffer = mapFileToMemory(argv[1]);
        
    // Output the file to the stdout,
    captureAndOutputFrames(mappedFrameBuffer, rate);

    return 0;
}