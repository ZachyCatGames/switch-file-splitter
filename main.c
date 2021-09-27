#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <windows.h>
#include <fileapi.h>

#define COPY_BUFFER_SIZE 0x4000000
#define SPLIT_FILE_SIZE 0xFFFF0000

int main(int argc, char* argv[]) {
    FILE* input_file;
    FILE* output_file;
    char* copy_buffer = (char*)malloc(COPY_BUFFER_SIZE);
    char out_path_name[MAX_PATH] = {0};
    const char* in_file_name = argv[1];

    /* Check arguments */
    if(argc != 2) {
        printf("Invalid arguments!\nUsage: whatever.exe path/to/file\n");
        return 69;
    }

    /* Setup out_path_name */
    snprintf(out_path_name, MAX_PATH, "split_%s", in_file_name);

    /* Make sure out_path_name doesn't already exist */
    if(GetFileAttributesA(out_path_name) != -1) {
        printf("Output path already exists!\n");
        return 3;
    }

    /* Create output directory */
    if(!CreateDirectoryA(out_path_name, NULL)) {
        printf("Failed to create directory %s!\n", out_path_name);
    }

    /* Open input file */
    if(fopen_s(&input_file, in_file_name, "rb")) {
        printf("Failed to open %s!\n", in_file_name);
        return 1;
    }

    /* Get file size */
    _fseeki64(input_file, 0, SEEK_END);
    const size_t input_file_size = _ftelli64(input_file);
    _fseeki64(input_file, 0, SEEK_SET);

    /* Calculate total split file ammount */
    const size_t split_count = (input_file_size + (SPLIT_FILE_SIZE - 1)) / SPLIT_FILE_SIZE;

    for(size_t i = 0; i < split_count; i++) {
        char split_file_name[MAX_PATH] = {0};
        size_t out_file_size = input_file_size - (SPLIT_FILE_SIZE * i) > SPLIT_FILE_SIZE ? SPLIT_FILE_SIZE : input_file_size - (SPLIT_FILE_SIZE * i);

        /* Setup split_file_name */
        snprintf(split_file_name, MAX_PATH, "%s\\%02zu", out_path_name, i);

        /* Open output file */
        if(fopen_s(&output_file, split_file_name, "wb")) {
            printf("Failed to open %s!\n", split_file_name);
            fclose(input_file);
            return 2;
        }
        
        while(out_file_size) {
            /* Determine amount of data to copy over this round */
            const size_t copy_size = COPY_BUFFER_SIZE < out_file_size ? COPY_BUFFER_SIZE : out_file_size;

            /* Read data from input into copy_buffer */
            fread(copy_buffer, 1, copy_size, input_file);
            
            /* Write to output file */
            fwrite(copy_buffer, 1, copy_size, output_file);

            /* Increment out_file_size */
            out_file_size -= copy_size;
        }
        
        /* Close output file */
        fclose(output_file);
    }

    const unsigned int folder_attributes = GetFileAttributesA(out_path_name);
    if(!SetFileAttributesA(out_path_name, folder_attributes | FILE_ATTRIBUTE_ARCHIVE)) {
        printf("Failed to set archive bit!\n");
    }

    free(copy_buffer);
    fclose(input_file);
}