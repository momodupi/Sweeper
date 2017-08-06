#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "camera.h"


int camera_open(FILE *fp)
{
    char rsltbuf[MAXLINE], command[MAXLINE], *result;
    
	snprintf(command, sizeof(command), "/home/pi/sweeper/camera.sh");
    fp = popen(command, "r");

    if (fp == NULL)
    {
        printf("popen fail!");
       	return -1;
    }

	//while (fgets(rsltbuf, sizeof(rsltbuf), fp) != NULL)
	{
		result = rsltbuf;
	}
	return 0;
}

int camera_close(FILE *fp)
{
	return pclose(fp);
}

