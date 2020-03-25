#define _XOPEN_SOURCE 500
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "string_lib.h"
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <memory.h>
#include <sys/times.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <libgen.h> 

#define MODE_SET       ((unsigned char) '1')
#define MODE_NOT_SET   ((unsigned char) '0')
#define MODE_MORE      ((unsigned char) '+')
#define MODE_LESS      ((unsigned char) '-')
#define MODE_PRECISELY ((unsigned char) '=')

#define PATH_MAX_SIZE 1000

// LOGGING ----------------------------------------------------------

void error(char* msg) {
    printf("Error: %s\n", msg);
    exit(0);
}

// -----------------------------------------------------------------

struct FindArgs {
    int mTime;
    unsigned char mMode;
    int aTime;
    unsigned char aMode;
    unsigned char maxDepthMode;
    int maxDepth;
} typedef FindArgs;

struct FileInfo {
    char* absolutePath;
    int   hardLinks;
    char* fileType;
    long int   sizeBytes;
    time_t accessTime;
    time_t modificationTime;
} typedef FileInfo;

FindArgs* args;

char* formatFileInfo(FileInfo* file) {
    char* infoBuffer = malloc(sizeof(char) * PATH_MAX_SIZE);
    char* accessTime = malloc(sizeof(char) * 26);
    char* modifiTime = malloc(sizeof(char) * 26);

    struct tm* tmAccess = localtime(&(file -> accessTime));
    struct tm* tmModif = localtime(&(file -> modificationTime));

    strftime(accessTime, 26, "%Y-%m-%d::%H:%M:%S", tmAccess);
    strftime(modifiTime, 26, "%Y-%m-%d::%H:%M:%S", tmModif);

    sprintf(infoBuffer, "%s %d %s %ldbytes %s %s",
                file -> absolutePath, file -> hardLinks, file -> fileType, 
                file -> sizeBytes, accessTime, modifiTime);

    free(accessTime);
    free(modifiTime);

    return infoBuffer;
}

int valid(FindArgs* args, FileInfo* info) {
    time_t currentTime = time(NULL);

    int mDiifD = (int) difftime(currentTime, info -> modificationTime) / (24 * 3600); 
    int aDiffD = (int) difftime(currentTime, info -> accessTime) / (24 * 3600);

    if (args -> mMode == MODE_PRECISELY && mDiifD != args -> mTime) {
        return 0;
    } else if (args -> mMode == MODE_MORE && mDiifD <= args -> mTime) {
        return 0;
    } else if (args -> mMode == MODE_LESS && mDiifD >= args -> mTime) {
        return 0;
    }

    if (args -> aMode == MODE_PRECISELY && aDiffD != args -> aTime) {
        return 0;
    } else if (args -> aMode == MODE_MORE && aDiffD <= args -> aTime) {
        return 0;
    } else if (args -> aMode == MODE_LESS && aDiffD >= args -> aTime) {
        return 0;
    }

    return 1;
}

void find(FindArgs* args, char* path, int depth) {
    if (args -> maxDepthMode == MODE_SET && (args -> maxDepth - depth < 0)) {
        return;
    }

    DIR* dir = opendir(path);

    if (!dir) {
        error("Failed to open directory.");
    }

    struct stat statBuffer;
    char* absPath  = (char*) malloc(sizeof(char) * PATH_MAX_SIZE);
    char* buffer   = (char*) malloc(sizeof(char) * (PATH_MAX_SIZE + 100));
    FileInfo* fileInfo = (FileInfo*) malloc(sizeof(FileInfo));

    realpath(path, absPath);

    for(struct dirent* dirent = readdir(dir); dirent != NULL; dirent = readdir(dir)) {
        char* filename = dirent -> d_name;
        if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
            continue;

        char* subpath = concatWithSeparator(absPath, filename, "/");
        stat(subpath, &statBuffer);

        fileInfo -> absolutePath = subpath;
        fileInfo -> accessTime = statBuffer.st_atime;
        fileInfo -> modificationTime = statBuffer.st_mtime;
        fileInfo -> sizeBytes = statBuffer.st_size;
        fileInfo -> hardLinks = statBuffer.st_nlink;

        if (S_ISREG(statBuffer.st_mode)) {
            fileInfo -> fileType = "file";
        } else if (S_ISDIR(statBuffer.st_mode)) {
            fileInfo -> fileType = "dir";
            lstat(subpath, &statBuffer);
            
            if (S_ISLNK(statBuffer.st_mode)) {
                fileInfo -> fileType = "slink";
            } else {
                find(args, subpath, depth + 1);
            }
        } else if (S_ISCHR(statBuffer.st_mode)) {
            fileInfo -> fileType = "char dev";
        } else if (S_ISBLK(statBuffer.st_mode)) {
            fileInfo -> fileType = "block dev";
        } else if (S_ISFIFO(statBuffer.st_mode)) {
            fileInfo -> fileType = "fifo";
        } else { // Socket
            fileInfo -> fileType = "sock";
        }
        
        char* info = formatFileInfo(fileInfo);
        
        if (valid(args, fileInfo)) {
            printf("%s \n", info);
        }

        free(fileInfo -> absolutePath);
        free(info);
    }

    free(fileInfo);    
    free(absPath);
    free(buffer);
}

// Find implementation usin nftw
static int nf(const char* fpath, const struct stat* sb, int tflag, struct FTW* ftwbuffer) {
    if (ftwbuffer -> level > args -> maxDepth || ftwbuffer -> level == 0) {
    //    return FTW_SKIP_SUBTREE | FTW_SKIP_SIBLINGS;
        return 0;
    }

    struct stat statBuffer = *sb;
    char* absPath  = (char*) malloc(sizeof(char) * PATH_MAX_SIZE);

    FileInfo* fileInfo = (FileInfo*) malloc(sizeof(FileInfo));
    fileInfo -> absolutePath = realpath(fpath, absPath);
    fileInfo -> accessTime = statBuffer.st_atime;
    fileInfo -> modificationTime = statBuffer.st_mtime;
    fileInfo -> sizeBytes = statBuffer.st_size;
    fileInfo -> hardLinks = statBuffer.st_nlink;
    
    if (tflag == FTW_D || tflag == FTW_DNR) {
        fileInfo -> fileType = "dir";
    } else if (tflag == FTW_F) {
        fileInfo -> fileType = "file";
    } else if (tflag == FTW_SL || tflag == FTW_SLN) {
        fileInfo -> fileType = "slink";

        char* dir = dirname(strdup(fpath));
        char* filename = basename(strdup(fpath));
        realpath(dir, absPath);
        char* cnc = concatWithSeparator(absPath, filename, "/");

        free(absPath);
        fileInfo -> absolutePath = cnc;
    } else if (tflag == FTW_NS) {
        fileInfo -> fileType = "cannot stat";
    }

    char* info = formatFileInfo(fileInfo);

    if (valid(args, fileInfo)) {
        printf("%s \n", info);
    }
    
    free(info);
    free(fileInfo -> absolutePath);
    free(fileInfo);

    return 0;
}

void findWithNFTW(char* path) {
    int flags = 1; // for ignoring symlinks

    nftw(path, nf, 20, flags);
}


// -----------------------------------------------------------------

int main(int argc, char* argv[]) {
    int i = 1;
    int nftw = 0;
    srand(time(0));

    args = (FindArgs*) malloc(sizeof(FindArgs));
    char*   path = argv[i];
    args -> aMode = MODE_NOT_SET;
    args -> mMode = MODE_NOT_SET;
    args -> maxDepthMode = MODE_NOT_SET; 

    while (++i < argc) {
        char* flag = argv[i];
        char* arg  = argv[++i];

        if (strcmp("-mtime", flag) == 0) {
            if (startsWith(arg, "-")) {
                char* strNum = suffix(arg, 1);
                int   value    = atoi(strNum);

                args -> mTime = value;
                args -> mMode = MODE_LESS;
                free(strNum);
            } else if (startsWith(arg, "+")) {
                char* strNum = suffix(arg, 1);
                int   value    = atoi(strNum);

                args -> mTime = value;
                args -> mMode = MODE_MORE;
                free(strNum);
            } else if (startsWithDigit(arg)) {
                int   value    = atoi(arg);

                args -> mTime = value;
                args -> mMode = MODE_PRECISELY;
            } else {
                error("Bad argument");
            }
        } else if (strcmp("-atime", flag) == 0) { 
            if (startsWith(arg, "-")) {
                char* strNum = suffix(arg, 1);
                int   value    = atoi(strNum);

                args -> aTime = value;
                args -> aMode = MODE_LESS;
                free(strNum);
            } else if (startsWith(arg, "+")) {
                char* strNum = suffix(arg, 1);
                int   value    = atoi(strNum);

                args -> aTime = value;
                args -> aMode = MODE_MORE;
                free(strNum);
            } else if (startsWithDigit(arg)) {
                int   value    = atoi(arg);

                args -> aTime = value;
                args -> aMode = MODE_PRECISELY;
            } else {
                error("Bad argument");
            }
        } else if (strcmp("-maxdepth", flag) == 0) { 
            args -> maxDepth = atoi(arg);
            args -> maxDepthMode = MODE_SET;
        } else if (strcmp("-nftw", flag) == 0) {
            nftw = 1;
        } else {
            error("Uknown flag");
        }
    }

    if (nftw) {
        findWithNFTW(path);
    } else {
        find(args, path, 1);
    }

    return 0;
}
