#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>

static char const alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                               "abcdefghijklmnopqrstuvwxyz"
                               "0123456789+/=";

#ifndef UINT8_MAX
#error "No support for uint8_t"
#endif

int main(int argc, char *argv[])
{

    int input_fd = STDIN_FILENO;
    int output_fd = STDOUT_FILENO;
    
    // Too many arguments provided in the command line
    if (argc > 2)
    {
        char errmsg[] = "Error: too many arguments; Correct example: './base64enc testfile'\n";
        write(output_fd, errmsg, sizeof errmsg);
        exit(EXIT_FAILURE);
    }
    // A file is provided as input; open it and point it to input_fd
    if (argc == 2 && strcmp("-", argv[1]) != 0)
    {
	    input_fd = open(argv[1], O_RDONLY);
    }

    while (1)
    {
        uint8_t in[3] = {0};        // input array
        uint8_t out[4];             // output array
        size_t nread = 0;           // holds number of bytes read
        static size_t count = 0;    // count of characters printed to console

        for (size_t offset = 0; offset < sizeof in;)
        {
            ssize_t n = read(input_fd, offset + (char *)in, sizeof in - offset);
            if (n < 0)
            {
                // error
                err(errno, "read failed");
            }
            if (n == 0)
            {
                // end of input and we have a partially filled input buffer
                break;
            }
            offset += n;
            nread = offset / sizeof *in;
        }
        if (nread == 0)
        {
            // Reached end of file *and* have NO input to process
            write(output_fd, "\n", 1);
            break;
        }

        // Bitwise manipulations
        out[0] = alphabet[in[0] >> 2];
        out[1] = alphabet[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        out[2] = alphabet[((in[1] & 0x0F) << 2) | (in[2] >> 6)];
        out[3] = alphabet[in[2] & 0x3F];
        // If the number of bytes read is less than 3, add padding
        for (size_t i = 3; i > nread; i--) out[i] = alphabet[64];
        count += nread + 1;

        for (size_t offset = 0; offset < sizeof out;)
        {
            ssize_t n = write(output_fd, offset + (char *)out, sizeof out - offset);
            if (n < 0)
            {
                // error
                err(errno, "write failed");
            }
            offset += n;
            // Add a newline when 76 characters have been printed to the console
            if (count == 76)
            {
                write(output_fd, "\n", 1);
                count = 0;
            }
        }
        if (nread < sizeof in / sizeof *in)
        {
            // Reached end of file and finished processing the last *partial* input
            write(output_fd, "\n", 1);
            break;
        }
    }
    return 0;
}
