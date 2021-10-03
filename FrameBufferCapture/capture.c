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

int main(int argc, char* argv[]) {
    
    int fileDescriptor;
    struct stat fileStat;
    size_t fileSize;

    // Make sure we have a filename argument,
    check(argc < 2, "Usage: ./capture.o <filename>\n");
    
    const char *file_name = argv[1];
    const char *mappedMemory;
    
    // Get file descriptor,
    fileDescriptor = open(file_name, O_RDONLY);
    check(fileDescriptor < 0, "open %s failed: %s", file_name, strerror(errno));

    // Get the size of the file,
    check(fstat(fileDescriptor, &fileStat) < 0, "stat %s failed: %s", file_name, strerror(errno));
    fileSize = fileStat.st_size;

    // Memory-map the file,
    mappedMemory = mmap(0, fileSize, PROT_READ, MAP_PRIVATE, fileDescriptor, 0);
    check(mappedMemory == MAP_FAILED, "mmap %s failed: %s", file_name, strerror(errno));

    // Output the file to the stdout,
    write(1 /*stdout*/, mappedMemory, fileSize);

    return 0;
}