#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define BUFFER_SIZE 999
int readable(char* input_path) {
    char directory_buffer[BUFFER_SIZE];
    struct dirent *directory_entry;
    struct stat stat_buffer;
    DIR *directory_pointer;
    int readable_file_count = 0;

    // Check if path exists and if not get the current working directory
    if (input_path == NULL) {
        if (getcwd(directory_buffer, sizeof(directory_buffer)) == NULL){
            fprintf(stderr, "getcwd() error: %s\n", strerror(errno));
            return errno*-1;
        }
    } else {
        // Else copy the input path over to directory buffer, prevent overflow by using buffer_size
        strncpy(directory_buffer, input_path, BUFFER_SIZE - 1);
        directory_buffer[BUFFER_SIZE - 1] = '\0'; 
    }

    // Check if the input path is a file and return if it's readable
    if (lstat(directory_buffer, &stat_buffer) == -1) {
        fprintf(stderr, "lstat() error: %s\n", strerror(errno));
        return errno*-1;
    }

    if (S_ISREG(stat_buffer.st_mode)) {
        int access_value;
        access_value = access(directory_buffer, R_OK);
        if (access_value < 0)
        {
            fprintf(stderr, "lstat() error: %s\n", strerror(errno));
            return errno*-1;
        }
        
        return 1;
    }

    // Otherwise, it's probably a directory, so open the directory
    directory_pointer = opendir(directory_buffer);
    if (directory_pointer == NULL) {
        fprintf(stderr, "opendir() error: %s\n", strerror(errno));
        return errno*-1;
    }

    // Loop through files in directory
    while ((directory_entry = readdir(directory_pointer)) != NULL) {

        // Skip file paths that would result in infinite loops
        if (strcmp(directory_entry->d_name, ".") == 0 || strcmp(directory_entry->d_name, "..") == 0) {
            continue;
        }

        // Construct the full path for recursion
        char full_path[BUFFER_SIZE];
        strncpy(full_path, directory_buffer, BUFFER_SIZE - 1);
        strncat(full_path, "/", BUFFER_SIZE - strlen(full_path) - 1);
        strncat(full_path, directory_entry->d_name, BUFFER_SIZE - strlen(full_path) - 1);

        // Call itself for recursion
        int recursion_result = readable(full_path);
        if (recursion_result >= 0) {
            readable_file_count += recursion_result;
        }
    }

    // Close the directory, prevent leaks
    if (closedir(directory_pointer) == -1) {
        fprintf(stderr, "closedir() error: %s\n", strerror(errno));
        return errno*-1;
    }

    // Return value
    return readable_file_count;
}




int main(int argc, char const *argv[])
{
    int count = readable(argv[1]);
    if (count >= 0) {
        printf("Number of readable files: %d\n", count);
    }
    return 0;
}

