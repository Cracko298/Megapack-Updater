#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "sha256.h" // Custom sha256 alg
#include "http_download.h" // Custom download alg

void sha256_hash(const uint8_t *data, size_t length, uint8_t *hash) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, length);
    sha256_final(&ctx, hash);
}

int main() {
    // Initialize services
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    fsInit();
    
    http_download("https://raw.githubusercontent.com/wyndchyme/mc3ds-modern/refs/heads/main/appConfiguration.info", "sdmc:/appConfiguration.info");

    // Open the file
    FILE *file = fopen("sdmc:/testFile.txt", "rb");
    if (!file) {
        printf("Failed to open file\n");
        svcSleepThread(5000000000LL);
        gfxExit();
        return 1;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Read the file into a buffer
    uint8_t *buffer = (uint8_t *)malloc(fileSize);
    if (!buffer) {
        printf("Failed to allocate memory\n");
        fclose(file);
        gfxExit();
        return 1;
    }
    fread(buffer, 1, fileSize, file);
    fclose(file);

    // Compute the SHA-256 hash
    uint8_t hash[32];
    sha256_hash(buffer, fileSize, hash);

    // Print the hash
    printf("SHA-256 hash: ");
    for (int i = 0; i < 32; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");

    svcSleepThread(5000000000LL);
    free(buffer);
    gfxExit();
    return 0;
}
