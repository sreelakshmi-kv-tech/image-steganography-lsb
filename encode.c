#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include"common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/* Read and validate Encode args from argv */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //check if the argv[2][0] = '.bmp'
    if(argv[2]!=NULL && strstr(argv[2],".bmp")) 
        encInfo->src_image_fname=argv[2];
    else
        return e_failure;
    
    //check if the argv[2][0] = '.bmp'
    if(argv[3]!=NULL)
        if(strstr(argv[3],".c") || strstr(argv[3],".h") || strstr(argv[3],".sh") || strstr(argv[3], ".txt")) //.c or .h or .sh or .txt
            encInfo->secret_fname=argv[3];
        else
            return e_failure;
    else
        return e_failure;        
    
    // Check for output stego image name     
    if(argv[4]!=NULL) 
        if(strstr(argv[4],".bmp"))
            encInfo -> stego_image_fname=argv[4]; 
        else
            return e_failure;
    else
        encInfo -> stego_image_fname="stego.bmp";
        
    return e_success;    
}

/* check capacity */
Status check_capacity(EncodeInfo *encInfo)
{
    //copy_bmp_header -> 54 byte
    //encode_magic_string -> 16 bytes
    //extn_size -> 32 byte
    //extn -> (2*8) or (3*8) or (4*8)
    //file_size -> 32 byte
    //secret data -> (24*8)

    // 1. Get image capacity in bytes
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    // 2. Get secret file size
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    // 3. Get secret file extension
    char *extn = strstr(encInfo->secret_fname, ".");
    if (extn == NULL)
    {
        printf("Error: Secret file has no extension\n");
        return e_failure;
    }

    int extn_len = strlen(extn);

    // 4. Calculate total bits required
    long total_bits_required = 0;

    total_bits_required = (16) +                    // Magic string
                          (32) +                    // Extension size
                          (extn_len * 8) +          // Extension itself
                          (32) +                    // File size
                          (encInfo->size_secret_file * 8);  // Secret file data

    // 5. Compare with image capacity
    // Each byte of image can store 1 bit in LSB
    if (total_bits_required > encInfo->image_capacity)
    {
        printf("Error: Image does not have enough capacity.\n");
        return e_failure;
    }

    return e_success;
}

/* Copy bmp image header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    //declare the char array with size is 54
    //rewind the src file
    //read the 54 bytes of data from the src file
    //write the 54 bytes of data in dest file

    char header[54];
    rewind(fptr_src_image);
    fread(header, 1, 54, fptr_src_image);
    fwrite(header, 1, 54, fptr_dest_image);
    if(ftell(fptr_src_image)==ftell(fptr_dest_image))
        return e_success;
    else 
        return e_failure;    
}

/* Encode a byte into LSB of image data array */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    //'#'-> 0 0 1 0 0 0 1 1 ->((data >> 7) & 1)[for getting msb bit]
    //0x01 -> 0 0 0 0 0 0 0 1 ->   1. clear&(~1) ->0x00[this is the updated value]
                                // 2. set | ((data >> 7) & 1)
                              // ((data>>6)&1)
    char bit_data;
    for(int i=7;i>=0;i--)      //for int i=31
    {
        bit_data=((data>>i)&1);
        image_buffer[7-i]=image_buffer[7-i] & (~1);
        image_buffer[7-i]=image_buffer[7-i] | bit_data;
    }
    return e_success;
}


/* Store Magic String */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    //declare the arr with size 8.
    //run the loop strlen(magic_string)(i=0;i<2 condition)
        //read the 8 byte of data from src file               //here reding and writing mention the same name
        //call the Status encode_byte_to_lsb(magic_string[i], char *image_buffer)
        //write the 8 bytes of data in dest file
    
    char arr[8];
    for (int i = 0; i < strlen(magic_string); i++)
    {
        fread(arr, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i], arr);
        fwrite(arr, 1, 8, encInfo->fptr_stego_image);
    }
    

    return e_success;    
}

/* Encode secret file extenstion size */
Status encode_secret_file_extn_size(const char* file_extn, EncodeInfo *encInfo)
{
      //rewind for fptr_secret 
    //declare the arr with size 32
    //read the 32 byte of data from src file
    //call the encode_int_to_lsb(strlrn(encInfo -> extn_secret_file),arr)
    //write the 32 byte data in lsb

    rewind(encInfo->fptr_secret);
    char arr[32];
    fread(arr, 1, 32, encInfo->fptr_src_image);
    encode_int_to_lsb(strlen(encInfo->extn_secret_file), arr);
    fwrite(arr, 1, 32, encInfo->fptr_stego_image);

    return e_success;
}

/* Encode secret file extenstion */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    //Declare the arr with size 8.
      //Run the loop strlen(file_extn)
          //Read the 8 byte of data from src file.
          //call the encode_byte_to_lsb(file_extn[i],arr)
          //write the 8 byte of data in dest file.
          
    char arr[8];
    for (int i = 0; i < strlen(file_extn); i++)
    {
        fread(arr, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], arr);
        fwrite(arr, 1, 8, encInfo->fptr_stego_image);
    }

    return e_success;      
}

/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    //Declare the arr with size 32
    //Read the 32 byte of data from src file.
    //call the encode_int_to_lsb(file_size,arr)
    //write the 32 byte of data in dest file.

    //Declare the arr with size 32
    char arr[32];

    //Read the 32 byte of data from src file.
    fread(arr, 1, 32, encInfo->fptr_src_image);

    //call the encode_int_to_lsb(file_size, arr)
    encode_int_to_lsb(file_size, arr);

    //write the 32 byte of data in dest file.
    fwrite(arr, 1, 32, encInfo->fptr_stego_image);

    return e_success;
}

/* Encode secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    //declare the arr with size 8.
    //run the loop encInfo -> size_secret_file
        //fread(encInfo -> secret_data,1,encInfo -> fptr_secret)
        //Read the 8 byte of data from src file. 
        //call the encode_byte_to_lsb(encInfo -> secret_data[0],arr)
        //write the 8 bytes of data in dest file.

    char arr[8];
    char ch;

    //rewind the secret file pointer to start
    rewind(encInfo->fptr_secret);
    
    for (long i = 0; i < encInfo->size_secret_file; i++)
    {
        //read one byte of data from secret file.
        fread(&ch, 1, 1, encInfo->fptr_secret);

        //Read the 8 byte of data from src file.
        fread(arr, 1, 8, encInfo->fptr_src_image);

        //call the encode_byte_to_lsb(encInfo -> secret_data[0], arr)
        encode_byte_to_lsb(ch, arr);

        //write the 8 bytes of data in dest file.
        fwrite(arr, 1, 8, encInfo->fptr_stego_image);
    }

    return e_success;
}

/* Get file size */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);       // Move pointer to end
    uint size = ftell(fptr);        // Get current position (file size)
    rewind(fptr);                   // Reset pointer back to start
    return size;                    // Return the size in bytes
}

/* Encode function, which does the real encoding */
Status encode_int_to_lsb(int size, char *image_buffer)
{
    for (int i = 0; i < 32; i++)
    {
        int bit = (size >> (31 - i)) & 1;   // extract bit (MSB first)
        image_buffer[i] = (image_buffer[i] & 0xFE) | bit;  // set LSB
    }
    return e_success;
}

/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1, fptr_src) == 1)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{
    printf("=====================================\n");
    printf("INFO: Starting Encoding Process...\n");
    printf("=====================================\n");

    // Step 1: Open files
    if (open_files(encInfo) != e_success)
    {
        printf("ERROR: Unable to open files.\n");
        return e_failure;
    }
    printf("INFO: Files opened successfully.\n");

    // Step 2: Check capacity
    if (check_capacity(encInfo) != e_success)
    {
        printf("ERROR: Image does not have enough capacity.\n");
        return e_failure;
    }
    printf("INFO: Image has enough capacity.\n");

    // Step 3: Copy BMP header
    if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        printf("ERROR: Failed to copy BMP header.\n");
        return e_failure;
    }
    printf("INFO: Header copied successfully.\n");

    // Step 4: Encode magic string
    if (encode_magic_string(MAGIC_STRING, encInfo) != e_success)
    {
        printf("ERROR: Failed to encode magic string.\n");
        return e_failure;
    }
    printf("INFO: Magic string encoded successfully.\n");

    // Step 5: Encode secret file extension size
    if (encode_secret_file_extn_size(encInfo->extn_secret_file, encInfo) != e_success)
    {
        printf("ERROR: Failed to encode file extension size.\n");
        return e_failure;
    }
    printf("INFO: File extension size encoded successfully.\n");

    // Step 6: Encode secret file extension
    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) != e_success)
    {
        printf("ERROR: Failed to encode file extension.\n");
        return e_failure;
    }
    printf("INFO: File extension encoded successfully.\n");

    // Step 7: Encode secret file size
    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) != e_success)
    {
        printf("ERROR: Failed to encode secret file size.\n");
        return e_failure;
    }
    printf("INFO: Secret file size encoded successfully.\n");

    // Step 8: Encode secret file data
    if (encode_secret_file_data(encInfo) != e_success)
    {
        printf("ERROR: Failed to encode secret data.\n");
        return e_failure;
    }
    printf("INFO: Secret data encoded successfully.\n");

    // Step 9: Copy remaining image data
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        printf("ERROR: Failed to copy remaining image data.\n");
        return e_failure;
    }
    printf("INFO: Remaining image data copied successfully.\n");

    printf("=====================================\n");
    printf("INFO: Encoding Completed Successfully!\n");
    printf("=====================================\n");

    return e_success;
}

/*
Status read_and_validate_decode_args(char *argv[], EncodeInfo *encInfo)
{
    printf("Decode part not implemented yet\n");
    return e_failure;
}
*/