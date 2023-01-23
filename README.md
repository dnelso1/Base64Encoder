# Base64Encoder
A C program that Base64 encodes a file or the standard input stream (`stdin`), then outputs the encoded text to the standard output stream (`stdout`).

base64encoder was compiled using gcc against the c99 standard
- `gcc -std=c99 -o base64enc base64enc.c`

## Usage
- `base64encoder` - Base64 encode input from `stdin`
- `base64encoder </path/to/file_name>` - Base64 encode a file
