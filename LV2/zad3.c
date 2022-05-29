#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

time_t min;
time_t defaultTimeFilter;
int bigFiles;

void listdir(char path, size_t size) {
    DIRdir;
    struct dirent entry;
    struct stat st;
    size_t len = strlen(path);
    char f[200];
    int sizeF;
    char date[10];

    if (!(dir = opendir(path))) {
        fprintf(stderr, "path not found: %s: %s\n",
                path, strerror(errno));
        return;
    }

    puts(path);
    while ((entry = readdir(dir)) != NULL) {
        charname = entry->d_name;
        if (entry->d_type == DT_DIR) {
            if (!strcmp(name, ".") || !strcmp(name, ".."))
                continue;
            if (len + strlen(name) + 2 > size) {
                fprintf(stderr, "path too long: %s/%s\n", path, name);
            } else {
                path[len] = '/';
                strcpy(path + len + 1, name);
                listdir(path, size);
                path[len] = '\0';
            }
        } else {
        strcpy(f, path);
        strcat(f, "/");
        strcat(f, name);
        stat(f, &st);
        sizeF = st.st_size;
        if(sizeF >= 10000000) ++bigFiles;
        if (min > st.st_ctime && st.st_ctime > defaultTimeFilter) {
            min = st.st_ctime;
            strftime(date, 20, "%d-%m-%y", localtime(&(st.st_ctime)));
                printf("%d, %s\n", sizeF, date);
        }
        }
    }

    closedir(dir);
}

int main(int argc, char* argv[]) {

   struct tm info;

   info.tm_year = 1971 - 1900;
   info.tm_mon = 1 - 1;
   info.tm_mday = 1;
   info.tm_hour = 0;
   info.tm_min = 0;
   info.tm_sec = 1;
   info.tm_isdst = -1;

   defaultTimeFilter = mktime(&info);


    char date[10];
    time(&min);
    bigFiles = 0;
    listdir(argv[1], 1024 * sizeof(char));
    strftime(date, 20, "%d-%m-%y", localtime(&(min)));
    printf("Number of big files: %d\nOldest file date: %s\n", bigFiles, date);

    struct dirent de;  // directory entry pointer

    // returns a DIR type pointer
    DIRdr = opendir(argv[1]);
    int fileCount = 0;
    int dirCount = 0; 

    if (dr == NULL)  // NULL if couldn't open directory 
    { 
        printf("Could not open current directory" ); 
        return 0; 
    } 

    while ((de = readdir(dr)) != NULL)
    { 
            printf("%s, %d\n", de->d_name, de->d_type);
        if (de->d_type == DT_DIR) ++dirCount;
        else if (de->d_type == DT_REG) ++fileCount;
    }

    closedir(dr);

    printf("Number of directories: %d\nNumber of files: %d\n", dirCount, fileCount);

    return 0; 
}