/*
Copyright (c) 2019 exotulip

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "gpiolib.h"
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <stdio.h>

int get_chipinfo(int chipfd,struct gpiochip_info* info){

	int status;
	status = ioctl(chipfd, GPIO_GET_CHIPINFO_IOCTL, info);
	if (status < 0){
		perror("get chipinfo error");
		return(-1);
	}
	return 0;
}


int get_lineinfo(int chipfd, int offset,struct gpioline_info* linfo){

	int status;
	linfo->line_offset = offset;
	status = ioctl(chipfd, GPIO_GET_LINEINFO_IOCTL, linfo);
	if (status < 0){
		perror("get lineinfo error");
		return(-1);
	}
	return 0;
}


struct gpioline init_line(int chipfd,int offset,unsigned int flags,int defval,char* consumer){
	struct gpioline line;
        line.req.lineoffsets[0] = offset;
        line.req.flags =  flags;
        line.req.default_values[0] = defval;
        strcpy(line.req.consumer_label,consumer);
        line.req.lines = 1;
	line.chipfd = chipfd;
	return(line);
}


int request_line(struct gpioline* line){
	int status;
        status = ioctl(line->chipfd, GPIO_GET_LINEHANDLE_IOCTL, &(line->req));
        if (status < 0){
                perror("get linehandle error");
		return(-1);
	}
        return(0);
}

int close_line(struct gpioline* line){
	int status;
	status = close(line->req.fd);
	if (status < 0){
		perror("line closing error");
		return(-1);
	}
	return(0);
}

int set_line(struct gpioline* line,int val){
	struct gpiohandle_data data;
	int status;
	data.values[0] = val;
	status = ioctl(line->req.fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);
	if (status < 0){
		perror("set value error");
		return(-1);
	}
	return(0);
}


int get_line(struct gpioline* line){
	struct gpiohandle_data data;
	int status;
	status = ioctl(line->req.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data);
	if (status < 0){
		perror("get value error");
		return(-1);
	}
	return(data.values[0]);
}

int change_line_output(struct gpioline* line){
	if(line->req.flags & GPIOHANDLE_REQUEST_INPUT){
		close_line(line);
		line->req.flags = GPIOHANDLE_REQUEST_OUTPUT;
        	int status;
        	status = ioctl(line->chipfd, GPIO_GET_LINEHANDLE_IOCTL, &(line->req));
        	if (status < 0){
                	perror("get linehandle error");
			return(-1);
		}
		return (0);
	}
	else
		return(0);
}

int change_line_input(struct gpioline* line){
        if(line->req.flags & GPIOHANDLE_REQUEST_OUTPUT){
                close_line(line);
                line->req.flags = GPIOHANDLE_REQUEST_INPUT;
                int status;
                status = ioctl(line->chipfd, GPIO_GET_LINEHANDLE_IOCTL, &(line->req));
                if (status < 0){
                        perror("get linehandle error");
			return(-1);
		}
                return(0);
        }
        else
                return(0);
}
