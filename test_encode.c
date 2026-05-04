/*
Name:Sreelakshmi kv
Start date:02/11/25
End date:12/11/25
Description:This project performs image steganography using LSB (Least Significant Bit) technique.
It hides and retrieves secret data from BMP images by modifying the least significant bits of pixel values.
*/
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "encode.h"
#include "decode.h"
#include "common.h"

/* Function to check operation type (-e / -d) */
OperationType check_operation_type(char *argv[])
{
    if (argv[1] == NULL)
        return e_unsupported;

    if (strcmp(argv[1], "-e") == 0)
        return e_encode;
    else if (strcmp(argv[1], "-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;

    /* Validation for minimum argument count */
    if (argc < 2)
    {
        printf("Usage:\n");
        printf("  For Encoding: ./a.out -e <.bmp> <.txt/.c/.h/.sh> [output.bmp]\n");
        printf("  For Decoding: ./a.out -d <stego.bmp> <output.txt/.c/.h/.sh>\n");
        return e_failure;
    }

    OperationType ret = check_operation_type(argv);

    /* ========================= ENCODING ========================= */
    if (ret == e_encode)
    {
        if (!(argc == 4 || argc == 5))
        {
            printf("ERROR: Invalid number of arguments for encoding!\n");
            printf("Usage: ./a.out -e <beautiful.bmp> <secret.txt/.c/.h/.sh> [output.bmp]\n");
            return e_failure;
        }

        // Validate input .bmp file
        if (!strstr(argv[2], ".bmp"))
        {
            printf("ERROR: Source image must be a .bmp file!\n");
            return e_failure;
        }

        // Validate secret file extension
        if (!(strstr(argv[3], ".txt") || strstr(argv[3], ".c") || strstr(argv[3], ".h") || strstr(argv[3], ".sh")))
        {
            printf("ERROR: Secret file must be .txt/.c/.h/.sh!\n");
            return e_failure;
        }

        // Proceed with encoding
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            if (open_files(&encInfo) == e_success)
            {
                printf("SUCCESS: open_files function completed\n");
                if (do_encoding(&encInfo) == e_success)
                    printf("INFO: Encoding completed successfully!\n");
                else
                    printf("ERROR: Encoding failed!\n");
            }
            else
                printf("ERROR: open_files function failed\n");
        }
        else
            printf("ERROR: Invalid arguments for encoding\n");
    }

    /* ========================= DECODING ========================= */
    else if (ret == e_decode)
    {
        if (!(argc == 3 || argc == 4))
        {
            printf("ERROR: Invalid number of arguments for decoding!\n");
            printf("Usage: ./a.out -d <stego.bmp> [output.txt/.c/.h/.sh]\n");
            return e_failure;
        }

        // Validate stego image
        if (!strstr(argv[2], ".bmp"))
        {
            printf("ERROR: Input stego image must be a .bmp file!\n");
            return e_failure;
        }

        // If output filename provided, validate it
        if (argc == 4)
        {
            if (!(strstr(argv[3], ".txt") || strstr(argv[3], ".c") || strstr(argv[3], ".h") || strstr(argv[3], ".sh")))
            {
                printf("ERROR: Output file must be .txt/.c/.h/.sh!\n");
                return e_failure;
            }
        }

        // Proceed with decoding
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            if (open_decode_files(&decInfo) == e_success)
            {
                printf("SUCCESS: open_decode_files function completed\n");
                if (do_decoding(&decInfo) == e_success)
                    printf("INFO: Decoding completed successfully!\n");
                else
                    printf("ERROR: Decoding failed!\n");
            }
            else
                printf("ERROR: open_decode_files function failed\n");
        }
        else
            printf("ERROR: Invalid arguments for decoding\n");
    }

    /* ========================= UNSUPPORTED ========================= */
    else
    {
        printf("ERROR: Unsupported operation.\n");
        printf("Use -e for encoding or -d for decoding.\n");
        printf("Usage:\n");
        printf("  ./a.out -e <.bmp> <.txt/.c/.h/.sh> [output.bmp]\n");
        printf("  ./a.out -d <stego.bmp> [output.txt/.c/.h/.sh]\n");
    }

    return 0;
}
