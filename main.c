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

#include "debug.h"
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <string.h>

/*
CAPABILITIES FUNCTIONS:

--erase-chip
--dump-flash output.bin
--flash-firmware input.bin
--read-id
--check-debug-lock
--set-debug-lock
--help

*/
int pins[3];

void help(){
	puts("Available Options:");
	puts("--read-id:\t\t\tRead chip ID.");
	puts("--check-debug-lock:\t\tCheck if the debug lock bit is set.");
	puts("--set-debug-lock:\t\tSet the debug lock bit.");
	puts("--erase-chip:\t\t\tErase whole flash memory.");
	puts("--dump-flash <output file>:\tDump whole flash content.");
	puts("--flash-firmware <input file>:\tWrite firmware file to chip. ");
	puts("--help:\t\t\t\tDisplay this help.");
}

int init(){

        if(programmer_init(pins[0],pins[1],pins[2])==-1){
		puts("programmer initialization error!");
		return(-1);
	}
        debug_init();
	return(0);
}

void erase(){

	if(init()==-1)
		return;

	unsigned char chip_id = read_chip_id();
	if(chip_id == 0) {
        	puts("No chip detected");
		programmer_close();
		return;
    	}

	puts("Requesting chip erase...");
	chip_erase();


	programmer_close();
}

void dump_flash(const char* filename){

	if(init()==-1)
		return;

	unsigned char chip_id = read_chip_id();
	if(chip_id == 0) {
        	puts("No chip detected");
		programmer_close();
		return;
    	}

	unsigned char status = read_status();
	if(status == 0) {
        	puts("No status received");
		programmer_close();
		return;
	}
	if(status & STATUS_DEBUG_LOCKED_BM){
		puts("Flash dump not possible. Debug lock set.");
		programmer_close();
		return;
	}

	FILE* file = fopen(filename,"a");
	if(file == NULL){
		perror("File opening error");
		programmer_close();
		return;
	}

	int len = 512;
	unsigned char buff[512];
	int addr = 0x0000;
	int bank = 0;
	puts("Downloading flash...");
	for(int i=0;i<512;++i){
		read_flash_memory_block(bank, addr, 512, buff);
		if(fwrite(buff,1,len,file)!=len){
			puts("error writing");
			break;
		}
		addr += 512;
		if(addr>=0x8000){
			addr = 0x0000;
			++bank;
		}
	}
	fclose(file);
	programmer_close();

}

void flash_firmware(const char* filename){

	if(init()==-1)
		return;

	unsigned char chip_id = read_chip_id();
	if(chip_id == 0) {
        	puts("No chip detected");
		programmer_close();
		return;
    	}

	FILE* file = fopen(filename,"r");
	if(file == NULL){
		perror("File opening error");
		return;
	}

	chip_erase();

	int len = 512;
	unsigned char buff[512];
	int addr = 0x0000;
	puts("Flashing in progress...");
	unsigned char debug_config = 0x22;

	debug_command(CMD_WR_CONFIG, &debug_config, 1);

	for(int i=0;i<512;++i){
		if(fread(buff,1,len,file)!=len){
			puts("error reading or EOF");
			break;
		}
		write_flash_memory_block(buff,addr, 512);
		addr += 512;
	}
	fclose(file);
	programmer_close();
}

void check_debug_lock(){

	if(init()==-1)
		return;

	unsigned char chip_id = read_chip_id();
	if(chip_id == 0) {
        	puts("No chip detected");
		programmer_close();
		return;
    	}

        puts("Reading status...");
	unsigned char status = read_status();
	if(status == 0) {
        	puts("No status received");
		programmer_close();
		return;
	}
	
	if(status & STATUS_DEBUG_LOCKED_BM)
		puts("DEBUG LOCK SET");
	else
		puts("DEBUG LOCK NOT SET");


	programmer_close();
}

void set_debug_lock(){

	if(init()==-1)
		return;

	unsigned char chip_id = read_chip_id();
	if(chip_id == 0) {
        	puts("No chip detected");
		programmer_close();
		return;
    	}

	unsigned char writebuf[4] = {0xff, 0xff, 0xff, 0x7f};
	unsigned char debug_config = 0x22;
	debug_command(CMD_WR_CONFIG, &debug_config, 1);
	puts("Setting debug lock bit...");
	write_flash_memory_block(writebuf,  0x3fffc, 4);
	programmer_close();
}


void readid(){

	if(init()==-1)
		return;


	puts("Reading chip id...");
	unsigned char chip_id = read_chip_id();
	if(chip_id == 0) {
        	puts("No chip detected");
		programmer_close();
		return;
    	}
        printf("Chip ID: %x\n",chip_id);


	programmer_close();
}



void main(int argc,char* argv[])
{
	puts("\n\t\t#############################");
	puts("\t\t#############################");
	puts("\t\t###   CC2541 programmer   ###");
	puts("\t\t#############################");
	puts("\t\t#############################");
	if(argc == 1){
		help();
		return;
	}	

	FILE* f = fopen("pins.cfg","r");
	if(f == NULL){
		perror("File opening error");
		return;
	}

	puts("\n\tReading config file...\n");
	char pin[3][11];
	for(int i=0;i<3;++i){
		fgets(pin[i],11,f);
		char* str = strtok(pin[i],":");
		if(strcmp(str,"DD")==0){
			pins[0]=atoi(strtok(NULL,":"));
			printf("\tData line number (%s) : %d\n",str,pins[0]);		
		}
		else if(strcmp(str,"DC")==0){
			pins[1]=atoi(strtok(NULL,":"));
			printf("\tClock line number (%s) : %d\n",str,pins[1]);		
		}
		else if(strcmp(str,"RESET_N")==0){
			pins[2]=atoi(strtok(NULL,":"));
			printf("\tReset line number (%s) : %d\n",str,pins[2]);	
		}	
		else{
			puts("Error in config file");
			return;
		}
	}
	fclose(f);

	nice(-20); /***sudo setcap 'CAP_SYS_NICE=ep' "$(pwd)/programmer"***/
	perror("\tSetting highest scheduler priority");
	printf("\tPriority: %d\n\n",getpriority(PRIO_PROCESS,0));	

	if(strcmp(argv[1],"--help")==0)
		help();
	else if(strcmp(argv[1],"--read-id")==0)
		readid();
	else if(strcmp(argv[1],"--erase-chip")==0){
		puts("Chip erase requested. Confirm by typing Y:");
		if(getc(stdin)=='Y')
			erase();
	}
	else if(strcmp(argv[1],"--check-debug-lock")==0)
		check_debug_lock();

	else if(strcmp(argv[1],"--set-debug-lock")==0){
		puts("Debug locking requested. Confirm by typing Y:");
		if(getc(stdin)=='Y')
			set_debug_lock();
		}
	else if(strcmp(argv[1],"--flash-firmware")==0){
		if(argc!=3){
			puts("Filename not specified.");	
			return;
		}
		puts("Flashing requested. All data will be lost. Confirm by typing Y:");
		if(getc(stdin)=='Y')
			flash_firmware(argv[2]);
	}
	else if(strcmp(argv[1],"--dump-flash")==0){
		if(argc!=3){
			puts("Filename not specified.");	
			return;
		}
		dump_flash(argv[2]);
	}
	else
		help();

}
