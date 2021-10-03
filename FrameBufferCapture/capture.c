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

int main(int argc, char* argv[]) {
    
    // Make sure we have a sufficient arguments,
    check(argc < 2, "Usage: ./capture.o <filename>\n");

    // Map file to memory,
    struct FileMemoryMapping mapping = mapFileToMemory(argv[1]);
        
    // Output the file to the stdout,
    write(1 /*stdout*/, mapping.memory, mapping.size);

    return 0;
}