/*
By downloading, copying, installing or using the software you agree to this license.
If you do not agree to this license, do not download, install,
copy or use the software.


                  License Agreement For libfacedetection
                     (3-clause BSD License)

Copyright (c) 2018-2020, Shiqi Yu, all rights reserved.
shiqi.yu@gmail.com

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are disclaimed.
In no event shall copyright holders or contributors be liable for any direct,
indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
*/

#include <stdio.h>
#include "facedetectcnn.h"
#include "detect-image.hh"
#include "generate_bitmap.h"

//define the buffer size. Do not change the size!
#define DETECT_BUFFER_SIZE 0x20000

// facedetection_callback facedetection_cb;

int yuv420_rgb(unsigned char *yuv, int width, int height)
{
  unsigned char *rgb = (unsigned char *)malloc(width * height * 3);
  YUV420SPToRGBByte(yuv, rgb, width, height, YUV420SP_NV21);
  facedetection(rgb, width, height);
  // save_rgb_bmp("face.bmp", rgb, width, height, 24);
  free(rgb);
  return 0;
}

int facedetection(unsigned char *ptr, int width, int height)
{
  // int width = WIDTH;
  // int height = HEIGHT;
  printf("facedetection...\n");
  int step = width * 3;

  int *pResults = NULL;
  //pBuffer is used in the detection functions.
  //If you call functions in multiple threads, please create one buffer for each thread!
  unsigned char *pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
  if (!pBuffer)
  {
    fprintf(stderr, "Can not alloc buffer.\n");
    return -1;
  }
  printf("cols:%d,rows:%d,step:%d\n", width, height, step);
  pResults = facedetect_cnn(pBuffer, ptr, width, height, step);

  printf("%d faces detected.\n", (pResults ? *pResults : 0));

  //print the detection results
  for (int i = 0; i < (pResults ? *pResults : 0); i++)
  {
    short *p = ((short *)(pResults + 1)) + 142 * i;
    int confidence = p[0];
    int x = p[1];
    int y = p[2];
    int w = p[3];
    int h = p[4];

    //print the result
    printf("face %d: confidence=%d, [%d, %d, %d, %d] (%d,%d) (%d,%d) (%d,%d) (%d,%d) (%d,%d)\n",
           i, confidence, x, y, w, h,
           p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14]);
  }

  //release the buffer
  free(pBuffer);

  return 0;
}
