#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // argv[0] = ./stego
    // argv[1] = -d
    // argv[2] = stego_img.bmp
    // argv[3] = output.txt

    if (argv[2] == NULL || argv[3] == NULL)
    {
        return e_failure;
    }

    decInfo->des_image_fname = argv[2];   // Stego image file (input)
    decInfo->output_fname = argv[3];      // Output file (decoded secret text)

    return e_success;
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char decoded_str[10]; // Assuming magic string is small (like "#*")
    char image_buffer[8];
    int i;

    // Decode each character of the magic string
    for (i = 0; i < strlen(magic_string); i++)
    {
        // Read 8 bytes (1 byte = 8 bits hidden in LSBs)
        fread(image_buffer, 8, 1, decInfo->fptr_des_image);
        char data[3];
        // Decode those 8 bits into 1 character
        decoded_str[i] = decode_byte_from_lsb(image_buffer);
        //data[2]='\0';
    }

    decoded_str[i] = '\0'; // Null terminate the decoded string
printf("hi-->%s\n",decoded_str);
    // Compare with original magic string
    if (strcmp(decoded_str, magic_string) == 0)
    {
        //printf("INFO: Magic string decoded successfully.\n");
        return e_success;
    }
    else
    {
        printf("ERROR: Magic string mismatch! Not a valid stego image.\n");
        return e_failure;
    }
}

Status decode_secret_file_extn_size(const char *file_extn, DecodeInfo *decInfo)
{
    char image_buffer[32]; // 32 bytes = 32 bits (LSB of each byte)
    int size = 0;

    // Read 32 bytes from the image (each LSB holds one bit of size)
    fread(image_buffer, 32, 1, decInfo->fptr_des_image);

    // Decode the 32 bits into an integer
    for (int i = 0; i < 32; i++)
    {
        size = (size << 1) | (image_buffer[i] & 1);
    }

    decInfo->size_output_file = size; // store in struct

    //printf("INFO: Secret file extension size decoded: %d\n", size);
    return e_success;
}

Status decode_secret_file_extn(const char *file_extn, DecodeInfo *decInfo)
{
    char image_buffer[8]; // each character is hidden in 8 bytes

    //printf("INFO: Decoding secret file extension...\n");

    // Loop through number of extension characters
    for (int i = 0; i < decInfo->size_output_file; i++)
    {
        // Read 8 bytes for one character
        fread(image_buffer, 8, 1, decInfo->fptr_des_image);

        char ch = 0;

        // Extract LSB bits to reconstruct the character
        for (int j = 0; j < 8; j++)
        {
            ch = (ch << 1) | (image_buffer[j] & 1);
          
        }

        decInfo->extn_output_file[i] = ch;
    }

    decInfo->extn_output_file[decInfo->size_output_file] = '\0'; // null terminate
    //printf("INFO: Secret file extension decoded: %s\n", decInfo->extn_output_file);

    return e_success;
}

Status open_decode_files(DecodeInfo *decInfo)
{
    // Open stego image for reading
    decInfo->fptr_des_image = fopen(decInfo->des_image_fname, "r");
    if (decInfo->fptr_des_image == NULL)
    {
        perror("fopen");
        printf("ERROR: Unable to open stego image file %s\n", decInfo->des_image_fname);
        return e_failure;
    }

    // Open output file for writing the decoded secret
    decInfo->fptr_output = fopen(decInfo->output_fname, "w");
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen");
        printf("ERROR: Unable to open output file %s\n", decInfo->output_fname);
        return e_failure;
    }

    return e_success;
}   

Status decode_secret_file_size(long file_size, DecodeInfo *decInfo)
{
    char image_buffer[32];// Buffer to store 32 bytes read from stego image 
    file_size = 0;// Initialize file_size to 0  

    fread(image_buffer, 1, 32, decInfo->fptr_des_image);// Read 32 bytes (bits for file size) from stego image  

    for (int i = 0; i < 32; i++)// Loop through each bit in the 32 bytes 
    {
        file_size = (file_size << 1) | (image_buffer[i] & 1); // Extract LSB and reconstruct file size bit by bit 
    }

    decInfo->size_output_file = file_size;// Store decoded file size in structure  
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;

    //printf("INFO: Decoding secret file data...\n");

    for (long i = 0; i < decInfo->size_output_file; i++)
    {
        ch = 0;

        // Read 8 bytes (each byte hides 1 bit of secret data)
        if (fread(image_buffer, 1, 8, decInfo->fptr_des_image) != 8)
        {
            printf("ERROR: Unable to read image data for secret file\n");
            return e_failure;
        }

        // Reconstruct 1 character from 8 LSBs
        for (int j = 0; j < 8; j++)
        {
            ch = (ch << 1) | (image_buffer[j] & 1);
        }

        // Write the decoded character to output file
        fputc(ch, decInfo->fptr_output);
    }

    //printf("INFO: Decoding secret file data completed successfully.\n");

    return e_success;
}

Status decode_int_from_lsb(int size, char *image_buffer)
{
    // simple decode of 32 bits from image_buffer
    size = 0;
    for (int i = 0; i < 32; i++)
    {
        size = (size << 1) | (image_buffer[i] & 1);// Extract LSB and reconstruct integer bit by bit  
    }
    return e_success;
}

char decode_byte_from_lsb(char *image_buffer)
{
  char  data = 0;
    for (int i = 0; i < 8; i++)
    {
        data = (data << 1) | (image_buffer[i] & 1);// Extract LSB from each byte and reconstruct character
    }
    printf("%c\n",data);
    return data;
}

Status skip_bmp_header(FILE *fptr_src_image)
{
    // char header[54];
    // fread(header, 1, 54, fptr_src_image);   // Read header
    // fwrite(header, 1, 54, fptr_dest_image); // Write to destination (optional)
    fseek(fptr_src_image,54,SEEK_SET);
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    printf("=====================================\n");
    printf("INFO: Starting Decoding Process...\n");
    printf("=====================================\n");

    // Step 1: Open stego image and output file
    if (open_decode_files(decInfo) == e_failure)
    {
        printf("ERROR: Unable to open files for decoding.\n");
        return e_failure;
    }

    // Step 2: Skip BMP header (first 54 bytes)
    skip_bmp_header(decInfo->fptr_des_image);
    printf("INFO: Skipped BMP header successfully.\n");

    // Step 3: Decode magic string
    if (decode_magic_string(MAGIC_STRING, decInfo) == e_failure)
    {
        printf("ERROR: Magic string mismatch. Not a valid stego image.\n");
        return e_failure;
    }
    printf("INFO: Magic string decoded successfully.\n");

    // Step 4: Decode secret file extension size
    if (decode_secret_file_extn_size(decInfo->extn_output_file, decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode secret file extension size.\n");
        return e_failure;
    }
    printf("INFO: Secret file extension size decoded successfully.\n");

    // Step 5: Decode secret file extension
    if (decode_secret_file_extn(decInfo->extn_output_file, decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode secret file extension.\n");
        return e_failure;
    }
    printf("INFO: Secret file extension decoded successfully.\n");

    // Step 6: Decode secret file size
    if (decode_secret_file_size(decInfo->size_output_file, decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode secret file size.\n");
        return e_failure;
    }
    printf("INFO: Secret file size decoded successfully.\n");

    // Step 7: Decode secret file data
    if (decode_secret_file_data(decInfo) == e_failure)
    {
        printf("ERROR: Failed to decode secret file data.\n");
        return e_failure;
    }
    printf("INFO: Secret file data decoded successfully.\n");

    printf("=====================================\n");
    printf("INFO: Decoding Completed Successfully!\n");
    printf("=====================================\n");

    return e_success;
}
