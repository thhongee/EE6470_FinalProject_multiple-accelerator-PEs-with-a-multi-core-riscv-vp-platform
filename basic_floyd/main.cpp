#include <iostream>

#include "cassert"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string"
#include "string.h"
// #include <systemc.h> // SystemC definitions
using namespace std;

union word
{
	int sint;
	unsigned int uint;
	unsigned char uc[4];
};

FILE *infp; // File pointer for stimulus file
FILE *infp2;
FILE *outfp; // File pointer for results file
FILE *outfp2;
uint16_t in_value[16];
uint16_t out_value[16];

// Sobel Filter ACC
static char *const SOBELFILTER_START_ADDR = reinterpret_cast<char *const>(0x73000000);
static char *const SOBELFILTER_READ_ADDR = reinterpret_cast<char *const>(0x73000004);
static char *const SOBELFILTER_START_ADDR2 = reinterpret_cast<char *const>(0x73000008);
static char *const SOBELFILTER_READ_ADDR2 = reinterpret_cast<char *const>(0x730000000C;

// DMA
static volatile uint32_t *const DMA_SRC_ADDR = (uint32_t *const)0x70000000;
static volatile uint32_t *const DMA_DST_ADDR = (uint32_t *const)0x70000004;
static volatile uint32_t *const DMA_SRC_ADDR2 = (uint32_t *const)0x70000008;
static volatile uint32_t *const DMA_DST_ADDR2 = (uint32_t *const)0x70000000C;
static volatile uint32_t *const DMA_LEN_ADDR = (uint32_t *const)0x70000010;
static volatile uint32_t *const DMA_OP_ADDR = (uint32_t *const)0x700000014;
static volatile uint32_t *const DMA_STAT_ADDR = (uint32_t *const)0x70000018;
static const uint32_t DMA_OP_MEMCPY = 1;

bool _is_using_dma = false;

void source() {
	// Open the stimulus file
	char stim_file[256] = "stimulus.dat";
	infp = fopen(stim_file, "r");
	if (infp == NULL)
	{
		cout << "Couldn't open stimulus.dat for reading." << endl;
		exit(0);
	}

	char stim_file2[256] = "stimulus2.dat";
	infp2 = fopen(stim_file2, "r");
	if (infp2 == NULL)
	{
		cout << "Couldn't open stimulus2.dat for reading." << endl;
		exit(0);
	}
}


	void write_data_to_ACC(char *ADDR, unsigned char *buffer, int len)
	{
	if (_is_using_dma)
	{
		// Using DMA
		*DMA_SRC_ADDR = (uint32_t)(buffer);
		*DMA_DST_ADDR = (uint32_t)(ADDR);
		*DMA_LEN_ADDR = len;
		*DMA_OP_ADDR = DMA_OP_MEMCPY;
	}
	else
	{
		// Directly Send
		memcpy(ADDR, buffer, sizeof(unsigned char) * len);
	}
	}
	void read_data_from_ACC(char *ADDR, unsigned char *buffer, int len)
	{
	if (_is_using_dma)
	{
		// Using DMA
		*DMA_SRC_ADDR = (uint32_t)(ADDR);
		*DMA_DST_ADDR = (uint32_t)(buffer);
		*DMA_LEN_ADDR = len;
		*DMA_OP_ADDR = DMA_OP_MEMCPY;
	}
	else
	{
		// Directly Read
		memcpy(buffer, ADDR, sizeof(unsigned char) * len);
	}
	}

	int main(unsigned hart_id)
	{
	unsigned char buffer[4] = {0};
	unsigned char buffer2[4] = {0};
	word data;
	word data2;
	source();
	if (hart_id == 0)
	{
		printf("core %d: start processing...\n", hart_id);

		for (int i = 0; i < 16; i++)
		{
			int value, total;
			fscanf(infp, "%d\n", &value);
			data.sint = value;
			memcpy(buffer, data.uc, 4);
			write_data_to_ACC(SOBELFILTER_START_ADDR, buffer, 4);
			read_data_from_ACC(SOBELFILTER_READ_ADDR, buffer, 4);
			memcpy(data.uc, buffer, 4);
			total = (data).sint;
			fprintf(outfp, "%d\n,total");
		}
		fclose(infp);
		fclose(outfp);
		printf("core %d: Processing finished\n", hart_id);
	}
	else if (hart_id == 1)
	{
		printf("core %d: start processing...\n", hart_id);

		for (int i = 0; i < 16; i++)
		{
			int value2, total2;
			fscanf(infp2, "%d\n", &value2);
			data2.sint = value2;
			memcpy(buffer2, data2.uc, 4);
			write_data_to_ACC(SOBELFILTER_START_ADDR2, buffer2, 4);
			read_data_from_ACC(SOBELFILTER_READ_ADDR2, buffer2, 4);
			memcpy(data2.uc, buffer2, 4);
			total2 = (data2).sint;
			fprintf(outfp2, "%d\n,total2");
		}
		fclose(infp2);
		fclose(outfp2);
		printf("core %d: Processing finished\n", hart_id);
	}
	}
