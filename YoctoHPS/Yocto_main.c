/*
 * main.c
 *
 *  Created on: Mar 10, 2022
 *      Author: f2jamil
 */
/* COE838 - System-on-Chip
 * Lab 4 - Custom IP for HPS/FPGA Systems
 * main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_gpio.h"
#include "hps_0.h"

#define LW_SIZE 0x00200000
#define LWHPS2FPGA_BASE 0xff200000

volatile uint32_t *md5_control = NULL;
volatile uint32_t *md5_data = NULL;
int success, total;


double getExecTime(struct timespec start_time, struct timespec stop_time){
	double tmp, tmp2;
	//Calculation of elapsed time
	tmp = (stop_time.tv_sec - start_time.tv_sec)*1e9;
	tmp  += (stop_time.tv_nsec - start_time.tv_nsec);

	printf("Total time %lf nsec \n", tmp);
	return tmp;
}

void load_systemback(uint32_t * vals){
	int i, j, k;
	size_t size = sizeof(vals);
	uint32_t wen, unit, data[16], addr;
	printf("load system method\n");
	alt_write_word(md5_control+2, 0x1); //assert wen for unit
	//while(!(alt_read_word(md5_control+1) & 0x1));
	wen = alt_read_word(md5_control+1);
	printf("Writing system: [0x%08x]\n", wen);
		for(i = 0; i< 512; i++){
			j = i % 16;
			alt_write_word(md5_data+1, i); //address
			alt_write_word(md5_data, vals[15-j]); //input value
			data[j] = alt_read_word(md5_data +2); //read input
			addr = alt_read_word(md5_data+ 3); //read address
			if (j == 15){
				printf("Writing: [0x");
				for(k = 0; k < 16; k++)
					printf("%08x", data[k]);
				printf("] to Unit %d @ 0x%08x\n", (int)ceil(i/16.0), addr);
			}
		}
	//}
	printf("Write to system done. De-Asserting signal\n");
	alt_write_word(md5_control+2, 0);
}


void load_system(uint32_t * vals){
	int i, j;
	size_t size = sizeof(vals);
	uint32_t wen, unit, data, addr;
	printf("load system method\n");
	alt_write_word(md5_control+2, 0x1); //assert wen for unit
	//while(!(alt_read_word(md5_control+1) & 0x1));
	wen = alt_read_word(md5_control+1);
	printf("Writing system: [0x%08x]\n", wen);

	//for(j = 0; j < 32; j++){
	//	unit = (uint32_t)pow(2, j-1);
	//	unit = unit << 4;
		for(i = 0; i< 512; i++){
			j = i % 16;
			alt_write_word(md5_data+1, i); //address
			alt_write_word(md5_data, vals[j]); //input value
			data = alt_read_word(md5_data +2); //read input
			addr = alt_read_word(md5_data+ 3); //read address
			printf("Writing: [0x%08x] to Cell [0x%08x]\n", data, addr);
		}
	//}
	printf("Write done. Deasserting signal\n");
	alt_write_word(md5_control+2, 0);
}

void load_unit(uint32_t * vals, int a){
	int i;
	size_t size = sizeof(vals);
	uint32_t wen, unit, data, addr;
	//unit = (uint32_t)(pow(2, a-1));
	unit = a << 4;
	printf("load unit method\n");
	alt_write_word(md5_control+2, 0x1); //assert wen for unit
	//while(!(alt_read_word(md5_control+1) & 0x1));
	wen = alt_read_word(md5_control+1);
	printf("Writing unit %d: [0x%08x]\n", a , wen);

	for(i = 0; i< 16; i++){
		alt_write_word(md5_data+1, unit + i);
		alt_write_word(md5_data, vals[i]);
		data = alt_read_word(md5_data + 2);
		addr = alt_read_word(md5_data+3);
		printf("Writing: [0x%08x] to Unit [0x%08x]", data, addr);
	}
	printf("Write done. Deasserting signal\n");
	alt_write_word(md5_control+2, 0);
}

void reset_unit(int a){
	uint32_t rst, unit;
	unit = (uint32_t)(pow(2, a-1));
	printf("reset unit method\n");
	alt_write_word(md5_control+1, unit); //assert reset
	//while(!(alt_read_word(md5_control+1) & 0x1));
	rst = alt_read_word(md5_control+1);
	printf("Reset unit %d: [0x%08x]\n", a ,rst);

	printf("Reset done. Deasserting signal\n");
	//alt_write_word(md5_control+1, 0x0); //assert reset
	while((alt_read_word(md5_control+1) & unit));//deassert
	rst = alt_read_word(md5_control+1);
	printf("Reset: [0x%08x]\n", rst);
}

void reset_system(){
	uint32_t rst;
	printf("reset system method\n");
	alt_write_word(md5_control+1, 0xFFFFFFFF); //assert reset
	while(!(alt_read_word(md5_control+1) & 0xFFFFFFFF));
	rst = alt_read_word(md5_control+1);
	printf("reset system method\n");
	printf("Reset: [0x%08x]\n", rst);

	printf("Reset done. Deasserting signal\n");
	//alt_write_word(md5_control+1, 0x0); //assert reset
	while((alt_read_word(md5_control+1) & 0xFFFFFFFF));//deassert
	rst = alt_read_word(md5_control+1);
	printf("Reset: [0x%08x]\n", rst);
}

void copy_to_input(uint32_t a, uint32_t b){
	uint32_t word, word2, word3;
	//alt_write_word(md5_control + 2, 0x00000000); //no writing to memory
	alt_write_word(md5_data + 1, a); //give me next digest
	//word = alt_read_word(md5_data + 1);//nothing
	//word2 = alt_read_word(md5_data + 3);//addressin
	word3 = alt_read_word(md5_data + 19);//addressin
	//printf("Same add as write: [0x%08x], Addrin3:  [0x%08x], Addrin19: [0x%08x]\n", word, word2, word3);
	printf("Addrin: [0x%08x]\n",word3);
	//alt_write_word(md5_data+1, b); //write to unit

	//start conversion
	//if (b < 50){alt_write_word(md5_control, b);}
	alt_write_word(md5_control, b);
	//alt_write_word(md5_control, 0xFFFFFFFF);

	//double check that start was asserted
	//while(!(alt_read_word(md5_control) & b));

	printf("Start successful\n");
}

void copy_to_system(){

	alt_write_word(md5_control, 0xFFFFFFFF);
	printf("Start successful\n");
}



double copy_output(uint32_t a, uint32_t b, struct timespec start_time){//potentially send a and b here as well or read the start
	uint32_t word, unit, index, unit2, unit3, word1, word2, word3, op1, op2;
	struct timespec stop_time;
	double exectime;
	uint32_t digest0[4], digest1[4];
	int i = 0;
	alt_write_word(md5_control, 0);
	//wait for done
	printf("waiting for done\n");
	op1 = alt_read_word(md5_control+2);
	printf("Done: [0x%08x]\n", op1);
	while(!(alt_read_word(md5_control+2) & b));
	clock_gettime(CLOCK_REALTIME, &stop_time);//Get Stop Time

	printf("Computation Complete, writing output\n");
	unit3 = alt_read_word(md5_data+2);
	index = alt_read_word(md5_data+19);
	for (i = 0; i < 4; i++){
		alt_write_word(md5_data+1, index + i);
		digest0[i] = alt_read_word(md5_data);
	}
	for (i = 0; i < 4; i++){
		alt_write_word(md5_data+1, index + i + 4);
		digest1[i] = alt_read_word(md5_data);
	}
	printf("Unit digest 0:  A:0x%08x ,B: 0x%08x ,C: 0x%08x ,D: 0x%08x  \n" , digest0[3], digest0[2], digest0[1], digest0[0]);
	printf("Unit digest 1:  A:0x%08x ,B: 0x%08x ,C: 0x%08x ,D: 0x%08x  \n" , digest1[3], digest1[2], digest1[1], digest1[0]);
	op1 = alt_read_word(md5_control+2);
	printf("Done: [0x%08x]\n Expected: baebddf8 61d3eb27 14ba892c 2ad26682\n ", op1);
	//baebddf8 61d3eb27 14ba892c 2ad26682
	if(digest0[3] == 0xbaebddf8 || digest0[2] == 0x61d3eb27 || digest0[1] == 0x14ba892c || digest0[0] == 0x2ad26682){
		if(digest1[3] == 0xbaebddf8 || digest1[2] == 0x61d3eb27 || digest1[1] == 0x14ba892c || digest1[0] == 0x2ad26682){
			printf("[SUCCESSFUL]\n");
			success++;
		}
	}else{

			printf("[FAILED]\n");
	}
	total++;
	printf("------------------------------------------------\n");
	exectime = getExecTime(start_time, stop_time);
	return exectime;
}

double sys_output(struct timespec start_time){//potentially send a and b here as well or read the start
	uint32_t word, unit, index, unit2, unit3, word1, word2, word3, op1, op2;
	uint32_t digest0[128], digest1[16];
	double exectime;
	struct timespec stop_time;
	int i = 0, j = 0;
	alt_write_word(md5_control, 0);
	//wait for done
	printf("waiting for done\n");
	op1 = alt_read_word(md5_control+2);
	printf("Done: [0x%08x]\n", op1);
	while(!(alt_read_word(md5_control+2) & 0xFFFFFFFF));
	clock_gettime(CLOCK_REALTIME, &stop_time);//Get Stop Time

	printf("Computation Complete, writing output\n");
	unit3 = alt_read_word(md5_data+2);
	//index = alt_read_word(md5_data+19);
	for (i = 0; i < 128; i++){
		alt_write_word(md5_data+1, i);
		digest0[i] = alt_read_word(md5_data);
	}
	//printf("\n");
	/*for (i = 0; i < 4; i++){
		alt_write_word(md5_data+1, index + i + 4);
		digest1[i] = alt_read_word(md5_data);
	}*/
	for (i = 0; i < 128; i++){
		j++;
		printf("Iteration: %d\n", j);
		printf("Unit digest 0:  A:0x%08x ,B: 0x%08x ,C: 0x%08x ,D: 0x%08x  \n" , digest0[i+3], digest0[i+2], digest0[i+1], digest0[i]);
		printf("Done: [0x%08x]\n Expected: baebddf8 61d3eb27 14ba892c 2ad26682\n ", op1);
			//baebddf8 61d3eb27 14ba892c 2ad26682
		if(digest0[i+3] == 0xbaebddf8 || digest0[i+2] == 0x61d3eb27 || digest0[i+1] == 0x14ba892c || digest0[i] == 0x2ad26682){
			//if(digest1[3] == 0xbaebddf8 || digest1[2] == 0x61d3eb27 || digest1[1] == 0x14ba892c || digest1[0] == 0x2ad26682){
				printf("[SUCCESSFUL]\n");
				success++;
		}
		else{

				printf("[FAILED]\n");
		}
		i+=3;
	}
	total+=j;
	printf("------------------------------------------------\n");
	exectime = getExecTime(start_time, stop_time);
	return exectime;
}

int main(int argc, char **argv){
	int fd, i, k, b;
	struct timespec start_time, stop_time;
	double exectime_savg, exectime_p, exectime_s[16];
	char choice;


	uint32_t values [16] = {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0};
	uint32_t valuestb [16] = {0x00000000,  0x00000150, 0x00000000, 0x00000000,
							0x00000000, 0x00808533, 0xeff0be7c,0x4de99287,
							0x5c433348, 0x0b78dac4, 0x103f26be, 0xa3793c48,
							0xb9657582, 0xcb8b2c30, 0x13ab80bb, 0x01680208};
	void *virtual_base;
	success = 0; total = 0;

	//map address space of fpga for software to access here
	if((fd = open("/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

	virtual_base =  mmap( NULL, LW_SIZE, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, LWHPS2FPGA_BASE);

	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return(1);
	}

	//initialize the addresses
	md5_control = virtual_base + ((uint32_t)( MD5_CONTROL_0_BASE) );
	md5_data = virtual_base + ((uint32_t)(MD5_DATA_0_BASE));
	printf("Welcome to MD5 Encryption Program\n");
	printf("Enter p for parallel or s for serial or b for both\n");
	scanf(" %c", &choice);
	printf("------>Finished initializing HPS/FPGA system<-------\n");
	if(choice != 'p'){
		k = 0;
		b = 3;
		//alt_write_word(md5_control, 0xFFFFFFFF);
		reset_system();
		load_systemback(valuestb);
		exectime_savg = 0.0;
		//write the data dude

		for(i = 0; i < 16; i++){
		//j = 2i + 1;
		printf("---------------- Iteration %d ------------------\n", i + 1);
			copy_to_input((uint32_t)k, (uint32_t)b);
			clock_gettime(CLOCK_REALTIME, &start_time);//Get Stop Time
			exectime_s[i] = copy_output((uint32_t)k, (uint32_t)b, start_time);
			exectime_savg += exectime_s[i];
			printf("Exectime for iteration %lf nsec\n", exectime_s[i]);
			b = b*4;
			k = k +8;
		}

		printf("[TEST PASSED] %d/%d\n", success, total);

		printf("Average Execution Time Serial : %lf\n", exectime_savg/16.0);
	}
	if(choice != 's'){
		printf("\n\n\nParallel Exec:\n");
		k = 0;
		b = 3;
		//alt_write_word(md5_control, 0xFFFFFFFF);
		reset_system();
		reset_system();
		load_systemback(valuestb);
		copy_to_system();
		clock_gettime(CLOCK_REALTIME, &start_time);//Get Start Time
		//printf("---------------- Iteration %d ------------------\n", i + 1);
		exectime_p = sys_output(start_time);

		printf("[TEST PASSED] %d/%d\n", success, total);
		printf("Exectime Parallel %lf nsec\n", exectime_p);
	}


	if (choice != 'p'){printf("Exectime Serial %lf nsec\n", exectime_savg/16.0);}
	// clean up our memory mapping and exit
	if( munmap( virtual_base, LW_SIZE) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	close( fd );


	return 0;

}


