// album.c
// CS 58
// HW1: Digital Photo Album
// author junjguan@
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#define INPUT_LEN 1024
typedef struct AlbumItem_s {
    char origin[INPUT_LEN];
    char scale10[INPUT_LEN];
    char scale50[INPUT_LEN];
    char caption[INPUT_LEN];
} AlbumItem;
char** inputPhotos;
int albumSize = 0;
AlbumItem* album;
char directoryName[INPUT_LEN];
char cwd[INPUT_LEN];
char header[] = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"><title>a sample index.html</title>\
<style type=\"text/css\"></style></head><body cz-shortcut-listen=\"true\"><h1>a sample index.html</h1>\
Please click on a thumbnail to view a medium-size image\
<h2>";

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
    int output = dp[M * N + N + M];
    free(dp);
    dp = NULL;
    return output;
}

void parsePathToPhotos(char* path) {
    char fileName[INPUT_LEN];
    bzero(directoryName, INPUT_LEN);
    bzero(fileName, INPUT_LEN);

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
        printf("Find matching files:\n ");
        while (ep = readdir (dp)) {
            if(ep->d_name[0] != '.' 
                && stringWildcardMatch(ep->d_name, fileName)) {
                inputPhotos[albumSize] = (char*) malloc(sizeof(char) * INPUT_LEN);
                if(inputPhotos[albumSize] == NULL) {
                    fprintf(stderr, "Couldn't malloc memroy for %s \n", ep->d_name);
                } else {
                    strcpy(inputPhotos[albumSize], ep->d_name);
                    printf("\t%s\n", inputPhotos[albumSize]);
                    if(++albumSize >= INPUT_LEN) {
                        printf("Reaching the boundary of 1024 inputPhotos in one time, ignoring the rest of them.\n");
                        break;
                    }
                }
            }
        }
        (void) closedir (dp);
    }
    else {
        fprintf(stderr, "Couldn't open the directory.\n");
    }
}

void displayImages(char* file) {
    char cmd[INPUT_LEN * 2];
    
    printf("Displaying %s, might take a few seconds... \n", file);
    int pid = fork();
    if(pid == -1) {
        fprintf(stderr, "Fork fails.\n");
    } else if (pid > 0) {
        return pid;
    } else {
        char cmd[] = "/usr/bin/display";
        execl (cmd, cmd, file, NULL);
        fprintf(stderr, "Error when execl(%s) with errno: %s \n", cmd, strerror(errno));
        _exit (EXIT_FAILURE);
    }    
}

void generateNewFileName(char* output, char* fileName, char* msg) {
    char extend[INPUT_LEN], tmp[INPUT_LEN];
    bzero(output, INPUT_LEN);
    bzero(extend, INPUT_LEN);
    bzero(tmp, INPUT_LEN);
    
    printf("Input filename is: %s\n", fileName);
    char* extendPointer = strrchr(fileName, '.');
    if(extendPointer != NULL)
        strcpy(extend, extendPointer);
    strcpy(tmp, cwd);
    strncat(tmp, fileName, extendPointer - fileName);
    strcat(tmp, msg);
    strcat(tmp, extend);
    printf("New file name is: %s\n", tmp);
    strcpy(output, tmp);
}

void doRotate(char* output, int idx, char*  degree) {
    char tmp[INPUT_LEN];
    printf("Rotate input is: %s\n", output);
    strcpy(tmp, output);
    char cmd[INPUT_LEN * 2];
    generateNewFileName(output, strrchr(tmp, '/'), ".rotate");
    sprintf(cmd, "convert -rotate %s %s %s", degree, tmp, output);
    printf("About to execute: %s\n", cmd);
    int pid = fork();
    if(pid == -1) {
        fprintf(stderr, "Fork fails.\n");
    } else if (pid > 0) {
        return pid;
    } else {
        char cmd[] = "/usr/bin/convert";
        execl (cmd, cmd, "-rotate", degree, tmp, output, NULL);
        fprintf(stderr, "Error when execl(%s) with errno: %s \n", cmd, strerror(errno));
        _exit (EXIT_FAILURE);
    }
}

void rotateImages(int idx) {
    // Get user degreetructions
    char degree[INPUT_LEN * 2];
    bzero(degree, INPUT_LEN * 2);
    while(1) {
        printf("How many degree do you want to roate the image? (type in -360 ~ 360, 0 by default):");
        int fetched = fgets(degree, 5, stdin);
        if(strlen(degree) <= 5) { 
            break;
        } else {
            puts("Input too long, try again...\n");
        }
    }
    degree[strlen(degree) - 1] = '\0';
    if(strlen(degree) == 0) {
        strcpy(degree, "0");
    }

    // Rotate the image
    doRotate(album[idx].scale10, idx, degree);
    doRotate(album[idx].scale50, idx, degree);
}

void captioningImage(char* caption, int idx) {
    printf("Type in caption of this image (or skip captioning by simply hitting enter):");
    fgets(caption, INPUT_LEN - 1, stdin);
    if(strlen(caption) <= 1) {
        strcpy(album[idx].caption, "<No title>");
    } else {
        strcpy(album[idx].caption, caption);
    }
    printf("Captioning %dth done\n", idx);
}

int scaleImageOutof100(int idx, int percent) {
    char cmd[INPUT_LEN * 2], scale[INPUT_LEN], percString[INPUT_LEN];
    char* output;
    if(percent == 10) {
        output = album[idx].scale10;
    } else {
        output = album[idx].scale50;
    }
    char* file = album[idx].origin;

    sprintf(scale, ".scale%d", percent);
    sprintf(percString, "%d%%", percent);
    generateNewFileName(output, strrchr(album[idx].origin, '/'), scale);
    sprintf(cmd, "convert -geometry %s %s %s", percString, file, output);
    printf("About to execute: %s\n", cmd);
    int pid = fork();
    if(pid == -1) {
        fprintf(stderr, "Fork fails.\n");
    } else if (pid > 0) {
        return pid;
    } else {
        char cmd[] = "/usr/bin/convert";
        execl (cmd, cmd, "-geometry", percString, file, output, NULL);
        fprintf(stderr, "Error when execl(%s) with errno: %s \n", cmd, strerror(errno));
        _exit (EXIT_FAILURE);
    }
}

void generateHtml() {
    int i;
    char output[INPUT_LEN * albumSize * 2];
    FILE* fp = fopen("album.html", "w");
    if(fp != NULL) {
        fputs(header, fp);
        for(i = 0; i < albumSize; i++) {
            char item[INPUT_LEN];
            sprintf(item, "<h2>%s</h2>\
<a href=\"%s\"><img src=\"%s\" border=\"1\"></a>\
</body></html>", album[i].caption, album[i].scale50, album[i].scale10);
            fputs(item, fp);
        }    
        fclose(fp);
    }
}

int main(int argc, char * argv[]) {
    int i;
    inputPhotos = (char**) malloc(sizeof(char*) * INPUT_LEN);
    // Parse input with inputPhotos
    if(argc <= 1) {
        parsePathToPhotos("");
    } else {
        i = 1;
        for(; i < argc; i++ ) {
            parsePathToPhotos(argv[i]);
        }
    }
    album = (AlbumItem*) malloc(sizeof(AlbumItem) * albumSize);
   
    // Process each of the inputPhotos
    for(i = 0; i < albumSize; i++) {
        int pid1, pid2, stat1, stat2;
        printf("Processing image: %s\n", inputPhotos[i]);
        strcpy(album[i].origin, directoryName);
        strcat(album[i].origin, inputPhotos[i]);
        
        // Scale the image
        pid1 = scaleImageOutof100(i, 50);
        pid2 = scaleImageOutof100(i, 10);
    
        // Dsiplay thunmnail
        waitpid(pid1, &stat1, NULL);
        waitpid(pid2, &stat2, NULL);
        displayImages(album[i].scale10);

        // Rotate if needed
        rotateImages(i);
        
        // Caption it if needed
        captioningImage(album[i].caption, i);
    }

    // Generate html file
    generateHtml();

    // Free the memory
    for(i = 0; i < albumSize; i++) {
        free(inputPhotos[i]);
    }
    free(album);
    free(inputPhotos);
    album = NULL;
    inputPhotos = NULL;
}
