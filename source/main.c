#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <curl/curl.h>

#define BASE_URL "https://raw.githubusercontent.com/wyndchyme/mc3ds-modern/main"
#define BASE_FILEPATH "sdmc:/luma/titles/%s/romfs"

bool fileExists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0;
}

bool downloadFile(const char *url, const char *outputPath) {
    FILE *fp = fopen(outputPath, "wb");
    if (!fp) {
        printf("Failed to open file for writing: %s\n", outputPath);
        return false;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Failed to initialize curl\n");
        fclose(fp);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);

    if (res != CURLE_OK) {
        printf("Failed to download file: %s\n", url);
        return false;
    }

    return true;
}

int main() {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    printf("Select your region:\n1. USA\n2. EUR\n3. JPN\n");

    u32 kDown;
    u8 region = 0;
    while (aptMainLoop()) {
        hidScanInput();
        kDown = hidKeysDown();

        if (kDown & KEY_A) {
            region = 1;
            break;
        } else if (kDown & KEY_B) {
            region = 2;
            break;
        } else if (kDown & KEY_X) {
            region = 3;
            break;
        }
    }

    if (region == 0) {
        printf("No region selected. Exiting...\n");
        gfxExit();
        return 0;
    }

    char *titleID;
    switch (region) {
        case 1:
            titleID = "0004000001670000"; // Example ID's until I can actually figure stuffs out... usa, eur, jpn in said order from up to down
            break;
        case 2:
            titleID = "0004000001670001";
            break;
        case 3:
            titleID = "0004000001670002";
            break;
    }

    char baseFilePath[256];
    snprintf(baseFilePath, sizeof(baseFilePath), BASE_FILEPATH, titleID);

    printf("Base filepath: %s\n", baseFilePath);

    mkdir(baseFilePath, 0777);

    const char *fileListURL = BASE_URL "/appConfiguration.info";
    const char *fileListPath = "/tmp/filelist.txt";

    if (!downloadFile(fileListURL, fileListPath)) {
        printf("Failed to download file list. Exiting...\n");
        gfxExit();
        return 1;
    }

    FILE *fileList = fopen(fileListPath, "r");
    if (!fileList) {
        printf("Failed to open file list. Exiting...\n");
        gfxExit();
        return 1;
    }

    char filePath[512];
    while (fgets(filePath, sizeof(filePath), fileList)) {
        filePath[strcspn(filePath, "\n")] = 0;

        char localPath[512];
        snprintf(localPath, sizeof(localPath), "%s/%s", baseFilePath, filePath);

        if (!fileExists(localPath)) {
            printf("File not found locally: %s\n", localPath);

            char remoteURL[512];
            snprintf(remoteURL, sizeof(remoteURL), "%s/%s", BASE_URL, filePath);

            printf("Downloading: %s\n", remoteURL);
            if (!downloadFile(remoteURL, localPath)) {
                printf("Failed to download %s\n", remoteURL);
            } else {
                printf("Successfully downloaded %s\n", filePath);
            }
        } else {
            printf("File exists locally: %s\n", localPath);
        }
    }

    fclose(fileList);

    printf("Operation complete. Press START to exit.\n");

    while (aptMainLoop()) {
        hidScanInput();
        kDown = hidKeysDown();
        if (kDown & KEY_START) break;
    }

    gfxExit();
    return 0;
}
