//
//  main.c
//  420P10ToP010
//
//  Created by Hank Lee on 10/12/15.
//  Copyright (c) 2015 Hank Lee. All rights reserved.
//

#include <stdint.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "pack.h"

#define MAX_WIDTH   7680
#define MAX_HEIGHT  4320


static uint8_t img[MAX_WIDTH * MAX_HEIGHT * 3];

int main(int argc, char *argv[])
{
    uint64_t *y;
    uint32_t *u;
    uint32_t *v;
    quatre_pixel q_pix;
    int ifd, ofd;

    int width;
    int height;
    int wxh;
    
    int i, j;
    int count = 0;
    ssize_t rd_sz;
    
    char *cp;
    char output[256] = { 0 };
    
    if (argc < 4)
    {
        fprintf(stderr, "usage: %s [input file] [width] [height]\n", argv[0]);
        
        return -1;
    }

    cp = NULL;

    // get input file name from comand line
    ifd = open(argv[1], O_RDONLY);
    if (ifd < 0)
    {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }

    // specify output file name
    cp = strrchr(argv[1], '.');
    strncpy(output, argv[1], cp - argv[1]);
    strcat(output, "_p010");
    strcat(output, cp);
    
    ofd = open(output, O_RDWR | O_CREAT, S_IRUSR);

    width   = atoi(argv[2]);
    height  = atoi(argv[3]);
    wxh     = width * height;

 
    while ((rd_sz = read(ifd, img, wxh * 3)) == wxh * 3)
    {
        y = (uint64_t *) img;
        u = (uint32_t *) ((uint8_t *) img + width * height * 2);
        v = (uint32_t *) ((uint8_t *) img + width * height * 2 + width * height * 2 / 4);

        // Y
        quatre_pixel *py = malloc(width * height / 4 * sizeof(quatre_pixel));
        quatre_pixel *p = py;
        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width / 4; j++) // 4 horizantal pixel at a time
            {
                memset(&q_pix, 0, sizeof(q_pix));

                pack_y(&q_pix, (uint8_t *) y);

                memcpy(p, &q_pix, sizeof(quatre_pixel));

                y++;
                p++;
            }
        }
        write(ofd, py, width * height / 4 * sizeof(quatre_pixel));
        free(py);
        
        // U, V
        quatre_pixel *quv = malloc(width * height / 2 / 4 * sizeof(quatre_pixel));
        quatre_pixel *q = quv;
        for (i = 0; i < height / 2; i++)
        {
            for (j = 0; j < width / 4; j++) // 2 horizontal pixel at a time
            {
                memset(&q_pix, 0, sizeof(q_pix));
                
                pack_uv
                (
                    &q_pix,
                    (uint8_t *) u,
                    (uint8_t *) v
                );

                memcpy(q, &q_pix, sizeof(quatre_pixel));

                u++;
                v++;
                q++;
            }
        }
        write(ofd, quv, width * height / 2 / 4 * sizeof(quatre_pixel));
        free(quv);
        
        fprintf(stderr, "Frame %d completed.\n", count);
        count++;
    }
    
    close(ifd);
    close(ofd);
    
    fprintf(stderr, "Done\n");
    fprintf(stderr, "Output file: %s\n", output);
    
    return 0;
}
