#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <3ds.h>
#include "http_download.h"

Result http_download(const char *url, const char *filename)
{
    Result ret=0;
    httpcContext context;
    char *newurl=NULL;
    u8* framebuf_top;
    u32 statuscode=0;
    u32 contentsize=0, readsize=0, size=0;
    u8 *buf, *lastbuf;
    FILE *file;

    ret = httpcInit(0x1000);
    if (ret != 0) { 
        printf("Failed to initialize HTTPC: %" PRId32 "\n", ret);
        return ret;
    }

    printf("Downloading %s\n", url);

    do {
		ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 1);
		printf("return from httpcOpenContext: %" PRId32 "\n",ret);

		// This disables SSL cert verification, so https:// will be usable
		ret = httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);
		printf("return from httpcSetSSLOpt: %" PRId32 "\n",ret);

		// Enable Keep-Alive connections
		ret = httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_ENABLED);
		printf("return from httpcSetKeepAlive: %" PRId32 "\n",ret);

		// Set a User-Agent header so websites can identify your application
		ret = httpcAddRequestHeaderField(&context, "User-Agent", "httpc-example/1.0.0");
		printf("return from httpcAddRequestHeaderField: %" PRId32 "\n",ret);

		// Tell the server we can support Keep-Alive connections.
		// This will delay connection teardown momentarily (typically 5s)
		// in case there is another request made to the same server.
		ret = httpcAddRequestHeaderField(&context, "Connection", "Keep-Alive");
		printf("return from httpcAddRequestHeaderField: %" PRId32 "\n",ret);

		ret = httpcBeginRequest(&context);
		if(ret!=0){
			httpcCloseContext(&context);
			if(newurl!=NULL) free(newurl);
			return ret;
		}

		ret = httpcGetResponseStatusCode(&context, &statuscode);
		if(ret!=0){
			httpcCloseContext(&context);
			if(newurl!=NULL) free(newurl);
			return ret;
		}

		if ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308)) {
			if(newurl==NULL) newurl = (char*)malloc(0x1000); // One 4K page for new URL
			if (newurl==NULL){
				httpcCloseContext(&context);
				return -1;
			}
			ret = httpcGetResponseHeader(&context, "Location", newurl, 0x1000);
			url = newurl; // Change pointer to the url that we just learned
			printf("redirecting to url: %s\n",url);
			httpcCloseContext(&context); // Close this context before we try the next
		}
	} while ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308));

    if(statuscode != 200){
        printf("URL returned status: %" PRId32 "\n", statuscode);
        httpcCloseContext(&context);
        if(newurl != NULL) free(newurl);
        return -2;
    }

    ret = httpcGetDownloadSizeState(&context, NULL, &contentsize);
    if(ret != 0){
        httpcCloseContext(&context);
        if(newurl != NULL) free(newurl);
        return ret;
    }

    printf("reported size: %" PRId32 "\n", contentsize);

    // Start with a single page buffer
    buf = (u8*)malloc(0x1000);
    if(buf == NULL){
        httpcCloseContext(&context);
        if(newurl != NULL) free(newurl);
        return -1;
    }

    do {
        ret = httpcDownloadData(&context, buf + size, 0x1000, &readsize);
        size += readsize; 
        if (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING){
            lastbuf = buf;
            buf = (u8*)realloc(buf, size + 0x1000);
            if(buf == NULL){ 
                httpcCloseContext(&context);
                free(lastbuf);
                if(newurl != NULL) free(newurl);
                return -1;
            }
        }
    } while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);    

    if(ret != 0){
        httpcCloseContext(&context);
        if(newurl != NULL) free(newurl);
        free(buf);
        return -1;
    }

    // Resize the buffer back down to our actual final size
    lastbuf = buf;
    buf = (u8*)realloc(buf, size);
    if(buf == NULL){
        httpcCloseContext(&context);
        free(lastbuf);
        if(newurl != NULL) free(newurl);
        return -1;
    }

    printf("downloaded size: %" PRId32 "\n", size);

    // Saving the file to the SD card
    file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Failed to open file for writing: %s\n", filename);
        httpcCloseContext(&context);
        free(buf);
        if (newurl != NULL) free(newurl);
        return -1;
    }

    fwrite(buf, 1, size, file);
    fclose(file);
    printf("File saved as %s\n", filename);

    httpcCloseContext(&context);
    free(buf);
    if (newurl != NULL) free(newurl);

    return 0;
}