#include <stdio.h>
#include <string.h>
#include "types.h"
#include "common.h"
#include "decode.h"

//opening the encoded image
Status open_files_decode(DecodeInfo *decInfo){
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname,"r");
    if(decInfo->fptr_stego_image == NULL){
        perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

    	return e_failure;
    }
}

//to decode character
int lsb_to_byte(char* image_buffer){
    char a = 0;
    for(int i=0;i<8;i++){
        char ch = image_buffer[i] & 0x01;
        a = a | (ch << (7-i));
    }
    return a;
}

//to decode integer
int lsb_to_int(char* image_buffer){
    int a = 0;
    for(int i=0;i<32;i++){
        char ch = image_buffer[i] & 0x01;
        a = a | (ch << (31-i));
    }
    return a;
}

//decode magic string
Status decode_magic_string(DecodeInfo *decInfo){
    int len = strlen(MAGIC_STRING);
    char str[len];
    //decoding magic string
    for(int i=0;i<len;i++){
        char buffer[8];
        fread(buffer,1,8,decInfo->fptr_stego_image);
        str[i] = lsb_to_byte(buffer);
    }

    char mag[len];
    printf("INFO: Enter the magic string:");
    scanf(" %s",mag);
    //checking the decoded magic string and the input magic string
    if(strcmp(str,mag) == 0){
        printf("INFO: Magic string verified\n");
        return e_success;
    }
    //if not verified stop
    else{
        printf("INFO: Magic string not verified\n");
        return e_failure;
    }
}

//decoding secret file extension size
Status decode_secret_file_extn_size(DecodeInfo *decInfo){
    char buffer[32];
    int ret = fread(buffer,1,32,decInfo->fptr_stego_image);
    decInfo->sec_file_ext_size = lsb_to_int(buffer);
    if(ret!=32){
        return e_failure;
    }
}

//decoding secret file extension
Status Decode_secret_file_extn(DecodeInfo *decInfo){
    int i;
    for(i=0;i<decInfo->sec_file_ext_size;i++){
        char buffer[8];
        fread(buffer,1,8,decInfo->fptr_stego_image);
        decInfo->extn_secret_file[i] = lsb_to_byte(buffer);
    }
    //copying null char to complete the string
    decInfo->extn_secret_file[i]='\0';
}

//opening the output file
Status Decode_open_output_file(DecodeInfo *decInfo){    
    //opening the outputfile
    decInfo->fptr_output = fopen(decInfo->output_fname,"w");
}

//decoding secret file size
Status Decode_secret_file_size(DecodeInfo *decInfo){   
    char buffer[32];
    fread(buffer,1,32,decInfo->fptr_stego_image);
    decInfo->size_secret_file = lsb_to_int(buffer);
}

//decoding secret file data
Status decode_secret_file_data(DecodeInfo *decInfo){
    for(int i=0;i<decInfo->size_secret_file;i++){
        char buffer[8];
        fread(buffer,1,8,decInfo->fptr_stego_image);
        char ch = lsb_to_byte(buffer);
        fputc(ch,decInfo->fptr_output);
    }
}

//start decoding
Status do_decoding(DecodeInfo *decInfo){
    //opening stego img
    printf("INFO: Opening required files\n");
    if(open_files_decode(decInfo) == e_failure){
        printf("INFO: Not done\n");
        return e_failure;
    }
    else{
        printf("INFO: Opened %s\n",decInfo->stego_image_fname);
    }

    //skipping header part
    fseek(decInfo->fptr_stego_image,54,SEEK_SET);

    //decode magic string
    printf("INFO: Decoding Magic String Signature\n");
    if(decode_magic_string(decInfo) == e_failure){
        printf("INFO: Not done\n");
        return e_failure;
    }
    else{
        printf("INFO: Done\n");
    }

    //decode secret file extension size
    decode_secret_file_extn_size(decInfo);

    //decode secret file extension
    printf("INFO: Decoding Output File Extension\n");
    Decode_secret_file_extn(decInfo);
    printf("INFO: Done\n");

    //copying the file name to structure array
    strcpy(decInfo->output_fname,decInfo->file_name);
    //combining the name with extension
    strcat(decInfo->output_fname,decInfo->extn_secret_file);

    // open the output file in write mode
    Decode_open_output_file(decInfo);
    if(decInfo->file_present == 1){
        printf("INFO: Output File not mentioned. Creating %s as default\n",decInfo->output_fname);
        printf("INFO: Opened %s\n",decInfo->output_fname);
    }
    else{
        printf("INFO: Opened %s\n",decInfo->output_fname);
    }
    printf("INFO: Done. Opened all required files\n");

    //decode secret file length
    printf("INFO: Decoding %s File Size\n",decInfo->output_fname);
    Decode_secret_file_size(decInfo);
    printf("INFO: Done\n");

    //decode secret file data
    printf("INFO: Decoding %s File Data\n",decInfo->output_fname);
    decode_secret_file_data(decInfo);
    printf("INFO: Done\n");

    return e_success;
}