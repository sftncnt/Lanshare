#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>


void cleanup_and_exit(FILE *fp, int sockfd) {
    if (fp) {
        fclose(fp);
    }
    if (sockfd) {
        close(sockfd);
    }
    exit(1);
}

FILE *get_file(char *filepath) {
    struct stat st;

    if (stat(filepath, &st) == -1) {
        perror("stat");
        return NULL;
    }

    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\n");
        return NULL;
    }
    
    printf("%s\n", filepath);
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        perror("open");
        return NULL;
    }
    return fp;
}

char *get_file_name(char *filepath) {
    char *name = strrchr(filepath, '/');
    if (name != NULL) {
        name++;
    } else {
        return filepath;
    }
    printf("Name: %s\n", name);
    return name;

}

off_t get_file_size(char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        perror("stat");
        return -1;
    }
    if (st.st_size > FILE_SIZE_MAX) {
        fprintf(stderr, "File size too large");
        return -1;
    }
    return st.st_size;
}

FILE *open_file(char *filename) {
    char filepath[256];
    FILE *fp = NULL;
    while (fp == NULL) {
        printf("Enter a valid path to save file to: ");
        fgets(filepath, 255, stdin);
        filepath[strcspn(filepath, "\n")] = '\0';
        strcat(filepath, "/");
        strcat(filepath, filename);
        printf("Updated filepath: %s\n", filepath);
        fp = fopen(filepath, "wb");
    }
    return fp;
}

