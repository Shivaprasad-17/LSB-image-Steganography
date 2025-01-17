#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

//check src file is .bmp or not
Status read_and_validate_encode_args(char *argv, EncodeInfo *encInfo){
    if(strstr(argv,".bmp")){
        return e_success;
    }
    else{
        return e_failure;
    }
}

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
    //printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    //printf("height = %u\n", height);

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
    printf("INFO: Opening required files\n");
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }
    printf("INFO: Opened %s\n",encInfo->src_image_fname);

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }
    printf("INFO: Opened %s\n",encInfo->secret_fname);

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }
    printf("INFO: Opened %s\n",encInfo->stego_image_fname);
    printf("INFO: Done\n");


    // No failure return e_success
    return e_success;
}

//to get file size
uint get_file_size(FILE *fptr){
    //to take offset to end
    fseek(fptr,0,SEEK_END);
    //to tell the position of offset
    return ftell(fptr);
}

//to get file extension size
uint get_file_ext_size(char* name){
    //finding the . address
    name = strchr(name,'.');
    //calculating length from . to end
    int len = strlen(name);
    return len; 
}

uint get_size_all(EncodeInfo *encInfo){
    //to get magic string length
    int magic = strlen(MAGIC_STRING);
    //secret file extension size will be a integer value so 4 bytes
    int sec_ext_size = 4;
    //secret file extension length
    int sec_ext;
    char* fin = strchr(encInfo->secret_fname,'.');
    sec_ext = strlen(fin);
    //secret file size will be a integer value so 4 bytes
    int sec_data = 4;
    return (magic + sec_ext_size + sec_ext + sec_data);
}
    
Status check_capacity(EncodeInfo *encInfo){
    //header size
    int head = 54;
    //getting image size
    int img_size = get_image_size_for_bmp(encInfo->fptr_src_image);
    //getting file size
    int sec_size = get_file_size(encInfo->fptr_secret);
    //remaining size
    int rem_size = get_size_all(encInfo);
    //total size
    int total_size_data = head+(8*(sec_size+rem_size));

    //if src img size is less than secret file then we cannot encode complete data
    if(img_size < total_size_data){
        printf("Size of image is less than size of data!\n");
        return e_failure;
    }
    else{
        return e_success;
    }
}

//copying the header directly to stego
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image){
    //rewinding the file pointers
    rewind(fptr_src_image);
    rewind(fptr_dest_image);
    char buffer[54];
    fread(buffer,1,54,fptr_src_image);
    int ret = fwrite(buffer,1,54,fptr_dest_image);
    if(ret != 54){
        return e_failure;
    }
}

//for encoding character
void byte_to_lsb(char data, char* image_buffer){
    for(int i=0;i<8;i++){
        //clear lsb of image buffer
        image_buffer[i] = (image_buffer[i] & 0xFE);

        //get 1 bit from msb of data
        char ch = (uint)(data & (1<<7-i))>>(7-i);

        //merge the 1 bit msb data & merge buffer data
        image_buffer[i] = image_buffer[i] | ch;
    }
}

//for encoding integer
void int_to_lsb(int data, char* image_buffer){
    for(int i=0;i<32;i++){
        //clear lsb of image buffer
        image_buffer[i] = (image_buffer[i] & 0xFE);

        //get 1 bit from msb of data
        char ch = (uint)((data & (1<<31-i)))>>(31-i);

        //merge the 1 bit msb data & merge buffer data
        image_buffer[i] = image_buffer[i] | ch;
    }
}

//encoding magic string
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo){
    if(magic_string == NULL){
        return e_failure;
    }
    for(int i=0;magic_string[i]!=0;i++){
        char buffer[8];
        fread(buffer,1,8,encInfo->fptr_src_image);
        byte_to_lsb(magic_string[i],buffer);
        fwrite(buffer,1,8,encInfo->fptr_stego_image);
    }
}

//encoding secret file extension size
Status encode_secret_file_extn_size(int size,EncodeInfo *encInfo){
    if(size == 0){
        return e_failure;
    }
    char buffer[32];
    fread(buffer,1,32,encInfo->fptr_src_image);
    int_to_lsb(size, buffer);
    fwrite(buffer,1,32,encInfo->fptr_stego_image);
}

//encoding secret file extension
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo){
    file_extn = strchr(file_extn,'.');
     if(file_extn == NULL){
        return e_failure;
    }
    for(int i=0;file_extn[i]!=0;i++){
    char buffer[8];
    fread(buffer,1,8,encInfo->fptr_src_image);
    byte_to_lsb(file_extn[i],buffer);
    fwrite(buffer,1,8,encInfo->fptr_stego_image);
    }
}

//encoding secret file size
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo){
    if(file_size == 0){
        return e_failure;
    }
    char buffer[32];
    fread(buffer,1,32,encInfo->fptr_src_image);
    int_to_lsb(file_size, buffer);
    fwrite(buffer,1,32,encInfo->fptr_stego_image);
}

//encoding secret file data
Status encode_secret_file_data(EncodeInfo *encInfo){
    rewind(encInfo->fptr_secret);
    char ch;
    while ((ch = fgetc(encInfo->fptr_secret)) != EOF){
        char buffer[8];
        fread(buffer,1,8,encInfo->fptr_src_image);
        byte_to_lsb(ch,buffer);
        fwrite(buffer,1,8,encInfo->fptr_stego_image);
    }
}

//coyping remaining data to complete the image
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    uint ch;
    while((ch = fgetc(fptr_src)) != EOF)
    {
        fputc(ch, fptr_dest);
    }
    return e_success;

    /*
    char buffer[8];
    int ret=1;
    while(ret!=0){
    ret=fread(buffer,1,1,fptr_src);
    fwrite(buffer,1,1,fptr_dest);
    }
    */
}

Status do_encoding(EncodeInfo *encInfo){
    //open file function
    if(open_files(encInfo) == e_failure){
        return e_failure;
    }

    printf("INFO: ## Encoding Procedure Started ##\n");
    printf("INFO: Checking for %s size\n",encInfo->secret_fname);
    printf("INFO: Done. ");

    //if file size is empty
    if(get_file_size(encInfo->fptr_secret)==0){
        printf("Empty\n");
        return e_failure;
    }
    else{
        printf("Not Empty\n");
    }

    //check capacity function
    printf("INFO: Checking for %s capacity to handle %s\n",encInfo->src_image_fname,encInfo->secret_fname);
    printf("INFO: Done. ");
    if(check_capacity(encInfo) == e_failure){
        printf("Found not OK\n");
        return e_failure;
    }
    else{
        printf("Found OK\n");
    }

    //copy bmp header
    printf("INFO: Copying Image Header\n");
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure){
        printf("INFO: Not done\n");
        return e_failure;
    }
    else{
        printf("INFO: Done\n");
    }

    //encode magic string
    printf("INFO: Encoding Magic String Signature\n");
    if(encode_magic_string(MAGIC_STRING,encInfo) == e_failure){
        printf("INFO: Not done\n");
        return e_failure;
    }
    else{
        printf("INFO: Done\n");
    }

    //encode secret file extension size
    if(encode_secret_file_extn_size(get_file_ext_size(encInfo->secret_fname),encInfo) == e_failure){
        return e_failure;
    }
    
    //encode secret file extension
    printf("INFO: Encoding %s File Extension\n",encInfo->secret_fname);
    if(encode_secret_file_extn(encInfo->secret_fname, encInfo) == e_failure){
        printf("INFO: Not done\n");
        return e_failure;
    }
    else{
        printf("INFO: Done\n");
    }

    //encode secret file size
    printf("INFO: Encoding %s File Size\n",encInfo->secret_fname);
    if(encode_secret_file_size(get_file_size(encInfo->fptr_secret), encInfo) == e_failure){
        printf("INFO: Not done\n");
        return e_failure;
    }
    else{
        printf("INFO: Done\n");
    }

    //encode secret file
    printf("INFO: Encoding %s File Data\n",encInfo->secret_fname);
    if(encode_secret_file_data(encInfo) == e_failure){
        printf("INFO: Not done\n");
        return e_failure;
    }
    else{
        printf("INFO: Done\n");
    }

    //copy remaining img data
    printf("INFO: Coping Left Over Data\n");
    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success){
        printf("INFO: Done\n");
    }
    else{
        printf("INFO: Not done\n");
        return e_failure;
    }
    
    return e_success;
}