#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4   

typedef struct _DecodeInfo
{
    /* Secret File Info */
    char output_fname[20];
    char* file_name;
    int file_present;
    FILE *fptr_output;
    int sec_file_ext_size;
    char extn_secret_file[MAX_FILE_SUFFIX]; 
    char secret_data[MAX_SECRET_BUF_SIZE];
    long size_secret_file;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} DecodeInfo;


/* Decoding function prototype */


/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Status open_files_decode(DecodeInfo *decInfo);

/* Get file size */
uint get_file_size(FILE *fptr);

/* Get file extension size */
uint get_file_ext_size(char* name);

/* Skip bmp image header */
Status skip_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

/* lsb to byte */
int lsb_to_byte(char* image_buffer);

/* lsb to int */
int lsb_to_int(char* image_buffer);

/* Decode Magic String and check*/
Status decode_magic_string(DecodeInfo *decInfo);

/*Decode the secret file extension size*/
Status decode_secret_file_extn_size(DecodeInfo *decInfo); 

/* Decode secret file extenstion */
Status Decode_secret_file_extn(DecodeInfo *decInfo);

/* Open the output file */
Status Decode_open_output_file(DecodeInfo *decInfo);

/* Encode secret file size */
Status Decode_secret_file_size(DecodeInfo *decInfo);

/* Encode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo);

#endif
