#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

static char const alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz"
                               "0123456789+/=";

#ifndef UINT8_MAX
#error "No support for uint8_t"
#endif

int main(int argc, char *argv[]) {
    FILE *input_fp = stdin;
    FILE *output_fp = stdout;
    
    // Too many arguments provided in the command line
    if (argc > 2) {
        char errmsg[] = "Too many arguments; Correct usage: './base64enc testfile' ";
        errno = 7;  // errno E2BIG - argument list too long
        perror(errmsg);
        exit(EXIT_FAILURE);
    }
    // A file is provided as input; open it and point it to input_fd
    if (argc == 2 && strcmp("-", argv[1]) != 0) {
	    input_fp = fopen(argv[1], "r");

        if (input_fp == NULL) {
            char errmsg[] = "Could not open input file";
            perror(errmsg);
            exit(EXIT_FAILURE);
        } else if (feof(input_fp)) {
            printf("End of file reached");
            exit(EXIT_FAILURE);
        }
    }

    while (1) {
        uint8_t in_buffer[3] = {0};   // input array
        uint8_t out_buffer[4];        // output array
        size_t bytes_read = 0;        // holds number of bytes read
        static size_t count = 0;      // count of characters printed to console

        for (size_t offset = 0; offset < sizeof in_buffer;) {
            // int buffer_len = sizeof in_buffer - offset;
            // size_t bytes = fread(offset + (char *)in_buffer, sizeof *in_buffer, buffer_len, input_fp);
            int buffer_len = sizeof in_buffer / sizeof *in_buffer;
            size_t bytes = fread(in_buffer, sizeof *in_buffer, buffer_len, input_fp);
                
            offset += bytes;
            bytes_read = offset / sizeof *in_buffer;

            if (feof(input_fp)) {
                // printf("End of file reached");
                break;
            }
            else if (ferror(input_fp)) {
                perror("Error reading input file");
                exit(EXIT_FAILURE);
            }
        }
        if (bytes_read == 0) {
            // Reached end of file and have NO input to process
            fwrite("\n", 1, 1, output_fp);
            break;
        }

        // Bitwise manipulations
        out_buffer[0] = alphabet[in_buffer[0] >> 2];
        out_buffer[1] = alphabet[((in_buffer[0] & 0x03) << 4) | (in_buffer[1] >> 4)];
        out_buffer[2] = alphabet[((in_buffer[1] & 0x0F) << 2) | (in_buffer[2] >> 6)];
        out_buffer[3] = alphabet[in_buffer[2] & 0x3F];
        
        // When the number of bytes read is less than 3, add padding
        for (size_t i = 3; i > bytes_read; i--) {
            out_buffer[i] = alphabet[64];
        }

        count += bytes_read + 1;
        for (size_t offset = 0; offset < sizeof out_buffer;) {
            // int buffer_len = sizeof out_buffer - offset;
            // size_t bytes = fwrite(offset + (char *)out_buffer, sizeof *out_buffer, buffer_len, output_fp);
            errno = 0;
            int buffer_len = sizeof out_buffer / sizeof *out_buffer;
            size_t bytes = fwrite(out_buffer, sizeof *out_buffer, buffer_len, output_fp);
            if (ferror(output_fp)) {
                perror("Error reading input file");
                exit(EXIT_FAILURE);
            }
            offset += bytes;
            // Add a newline when 76 characters have been printed to the console
            if (count == 76)
            {
                fwrite("\n", 1, 1, output_fp);
                count = 0;
            }
        }
        if (bytes_read < sizeof in_buffer / sizeof *in_buffer) {
            // Reached end of file and finished processing the last *partial* input
            fwrite("\n", 1, 1, output_fp);
            break;
        }
    }

    fclose(input_fp);
    return 0;
}
