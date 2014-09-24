// demo.c
//  CS 58
//  HW1: Digital Photo Album
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
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
    return output;
}

void parsePathToPhotos(char* path) {
    char directoryName[INPUT_LEN];
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
        printf("Find matching files:\n ");
        while (ep = readdir (dp)) {
            if(ep->d_name[0] != '.' 
                && stringWildcardMatch(ep->d_name, fileName)) {
                inputPhotos[albumSize] = (char*) malloc(sizeof(char) * INPUT_LEN);
                if(inputPhotos[albumSize] == NULL) {
                    fprintf(stderr, "Couldn't malloc memroy for %s \n", ep->d_name);
                } else {
                    strcpy(inputPhotos[albumSize], directoryName);
                    strcat(inputPhotos[albumSize], ep->d_name);
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
    printf("Displaying photo, might take a few seconds... \n");
    sprintf(cmd, "display %s", file);
    system(cmd);
}

void addStrBeforeExtend(char* output, char* input, char* msg) {
    char extend[INPUT_LEN], tmp[INPUT_LEN];
    bzero(output, INPUT_LEN);
    bzero(extend, INPUT_LEN);
    bzero(tmp, INPUT_LEN);
    char* extendPointer = strrchr(input, '.');
    if(extendPointer != NULL)
        strcpy(extend, extendPointer);
    strncat(tmp, input, extendPointer - input);
    strcat(tmp, msg);
    strcat(tmp, extend);
    strcpy(output, tmp);
}

void doRotate(char* file, char*  degree) {
    char tmp[INPUT_LEN];
    strcpy(tmp, file);
    char cmd[INPUT_LEN * 2];
    addStrBeforeExtend(file, tmp, ".rotate");
    sprintf(cmd, "convert -rotate %s %s %s", degree, tmp, file);
    printf("Executing: %s\n", cmd);
    system(cmd);
    puts("Rotation complete");
}

void rotateImages(char* file1, char* file2, char* file3) {
    // Get user degreetructions
    char degree[INPUT_LEN * 2];
    bzero(degree, INPUT_LEN * 2);
    while(1) {
        printf("How much do you want to roate the image? (type in -360 ~ 360, 0 by default):");
        int fetched = fgets(degree, 5, stdin);
        if(strlen(degree) <= 5) { 
            break;
        } else {
            puts("Input too long, try again...\n");
        }
    }
    degree[strlen(degree) - 1] = '\0';

    // Rotate the image
    doRotate(file1, degree);
    doRotate(file2, degree);
    doRotate(file3, degree);
}

void captioningImage(char* caption, int idx) {
    puts("Type in caption of this image (or skip captioning by simply hitting enter):");
    fgets(caption, INPUT_LEN - 1, stdin);
    printf("Captioning %dth done\n", idx);
}

void scaleImageOutof100(char* output, char* file, int percent) {
    char cmd[INPUT_LEN * 2], scale[INPUT_LEN];
    sprintf(scale, ".scale%d", percent);
    addStrBeforeExtend(output, file, scale);
    sprintf(cmd, "convert -geometry %d%% %s %s", percent, file, output);
    printf("Executing: %s\n", cmd);
    system(cmd);
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
        printf("Processing image: %s\n", inputPhotos[i]);
        strcpy(album[i].origin, inputPhotos[i]);
        
        // Scale the image
        scaleImageOutof100(album[i].scale50, inputPhotos[i], 50);
        scaleImageOutof100(album[i].scale10, inputPhotos[i], 10);
    
        // Dsiplay thunmnail
        displayImages(album[i].scale10);

        // Rotate if needed
        rotateImages(album[i].origin, album[i].scale50, album[i].scale10);
        
        // Caption it if needed
        captioningImage(album[i].caption, i);
    }
}
