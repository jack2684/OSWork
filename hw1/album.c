// demo.c
//  CS 58
//  HW1: Digital Photo Album
#include<string.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
const int INPUT_LEN = 1024;

char** photos;

int stringWildcardMatch(char* input, char* wildcard) {
    int N = strlen(input);
    int M = strlen(wildcard);
    int* dp = (int*) malloc (sizeof(int) * (N + 1) * (M + 1) );
    int i, j;
    
    // Init DP
    for(i = 0; i < M * N + M + N + 1; i++){
        dp[i] = 0;
    }
    dp[0] = 1;
    for(i = 1; i <= M; i++) {
        if(wildcard[i - 1] != '*') {
            break;        
        } else {
            dp[i * (N + 1)] = 1;
        }
    }
    for(i = 1; i <= M; i++) {
        for(j = 1; j <= N; j++) {
            int match = dp[(i - 1) * (N + 1) + j - 1] == 1 && (wildcard[i - 1] == '*' || wildcard[i - 1] == input[j - 1]);
            match |= dp[i * (N + 1) + j - 1] == 1 && wildcard[i - 1] == '*'; 
            match |= dp[(i - 1) * (N + 1) + j] == 1 && wildcard[i - 1] == '*';
            if(match)
                dp[i * (N + 1) + j ] = 1;
        }
    }
    return dp[M * N + N + M];
}

void parsePathToPhotos(char** photos, char* path) {
    char directoryName[INPUT_LEN];
    char fileName[INPUT_LEN];

    // Find the directory and the file name
    char* slash = strrchr(path, '/');
    char tmpDir[INPUT_LEN];
    int len = strlen(path);
    if(slash - path + 1 > INPUT_LEN) {
        fprintf(stderr, "Directory name too long.\n");
        return;
    }
    if(slash != NULL && len - (int)(slash - path) > INPUT_LEN) {
        fprintf(stderr, "File name too long: %d \n", (int)(slash - path) );
        return;
    }
    if(slash != NULL) {
        strncpy(tmpDir, path, slash - path);
        strcpy(fileName, slash + 1);
    } else if(slash == NULL && strlen(path)) {
        strcpy(fileName, path);
    }
    if(!strlen(fileName)) {
        strcpy(fileName, "*");
    }
    char cwd[INPUT_LEN];
    if (tmpDir[0] != '/' && getcwd(cwd, sizeof(cwd)) != NULL) {
        strcpy(directoryName, cwd);
        strcat(directoryName, "/");
        strcat(directoryName, tmpDir);
    } else {
        strcpy(directoryName, tmpDir);
    }
    int dirLen = strlen(directoryName);
    if(directoryName[dirLen - 1] != '/') {
        directoryName[dirLen] = '/';
        directoryName[dirLen + 1] = '\0';
    }
    printf("Directory is: %s \n", directoryName);
    printf("Filename is: %s \n", fileName);
    printf("---------------------------------------\n");

    // Add files to photo list
    DIR *dp;
    struct dirent *ep;
    dp = opendir (directoryName);
    if (dp != NULL) {
        while (ep = readdir (dp)) {
            if(ep->d_name[0] != '.' 
                && stringWildcardMatch(ep->d_name, fileName))
                printf("Find file: %s%s\n", directoryName, ep->d_name);
        }
        (void) closedir (dp);
    }
    else {
        fprintf(stderr, "Couldn't open the directory");
    }
}

int main(int argc, char * argv[]) {
    // Parse input with photos
    if(argc <= 1) {
        parsePathToPhotos(photos, "");
    } else {
        int i = 1;
        for(; i < argc; i++ ) {
            parsePathToPhotos(photos, argv[i]);
        }
    }
    // Dsiplay thunmnail

    // Rotate if needed
    
    // Caption it if needed

    // Generate half size
}
