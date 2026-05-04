#ifndef DECODE_H
#define DECODE_H

#include "types.h" 

//change macro name
#define MAX_DECODE_SECRET_BUF_SIZE 1
#define MAX_DECODE_IMAGE_BUF_SIZE (MAX_DECODE_SECRET_BUF_SIZE * 8)
#define MAX_DECODE_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Source Image info */
    char *des_image_fname;//storing output filename
    FILE *fptr_des_image;//output pointer
    //uint image_capacity;
    //uint bits_per_pixel;
    //char image_data[MAX_IMAGE_BUF_SIZE];

    /* Secret File Info */
    char *output_fname;
    FILE *fptr_output;

    char extn_output_file[MAX_DECODE_FILE_SUFFIX];
    char output_data[MAX_DECODE_SECRET_BUF_SIZE];
    long size_output_file;

    /* Stego Image Info */
    //char *stego_image_fname;
    //FILE *fptr_stego_image;

} DecodeInfo;


/* Encoding function prototype */

/* Check operation type */
//OperationType check_operation_type(char *argv[]);

/* Read and validate Encode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the encoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_decode_files(DecodeInfo *decInfo);

/* check capacity */
//Status check_capacity(EncodeInfo *encInfo);

/* Get image size */
//uint get_image_size_for_bmp(FILE *fptr_image);

/* Get file size */
//uint get_file_size(FILE *fptr);

/* Skip the 54-byte BMP header to reach the pixel data for decoding */
Status skip_bmp_header(FILE *fptr_src_image);

/* Store Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *encInfo);//change parameter also

//read extension size function call
Status decode_secret_file_extn_size(const char *file_extn, DecodeInfo *encInfo);//make a structure variable or global variable

/* Decode secret file extenstion */
Status decode_secret_file_extn(const char *file_extn, DecodeInfo *encInfo);//we need to change the extension .c to .txt after that open file

/* Decode secret file size */
Status decode_secret_file_size(long file_size, DecodeInfo *encInfo);

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *encInfo);

/* Decode function, which does the real encoding */
Status decode_int_from_lsb(int size,char *image_buffer);//32 bytes

/* Decode a byte into LSB of image data array */
char decode_byte_from_lsb( char *image_buffer);//32 bytes

/* Copy remaining image bytes from src to stego image after encoding */
//Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

//Status read_and_validate_decode_args(char *argv[], DecodeInfo *encInfo);

#endif
