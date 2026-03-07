#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <sys/types.h>
#include <openssl/sha.h>

#define FILE_SIZE_MAX 4000000000

void cleanup_and_exit(FILE *fp, int sockfd);

void print_sha256(unsigned char hash[SHA256_DIGEST_LENGTH]);

FILE *get_file(char *filepath);

char *get_file_name(char *filepath);

off_t get_file_size(char *path);

FILE *open_file(char *filename, char *return_filepath);


#endif
