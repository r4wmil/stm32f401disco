#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define CRC_IMPLEMENTATION
#include "crc.h"
#undef  CRC_IMPLEMENTATION

#define BINSER_IMPLEMENTATION
#include "binser.h"
#undef  BINSER_IMPLEMENTATION

#define UARTCOM_IMPLEMENTATION
#include "uartcom.h"
#undef UARTCOM_IMPLEMENTATION

#include "codes.h"

void send_test(uint8_t* d, size_t dsz) {
	uint8_t* buf = malloc(1 + 4 + dsz + 4);
	bswriter_t bsw = {.buf=buf,.top=0};
	bsw_putu8(&bsw, HOST_SOH);
	bsw_putu32l(&bsw, dsz);
	bsw_putbuf(&bsw, d, dsz);
	uint32_t hash = crc_32(bsw.buf + 1, bsw.top - 1);
	//printf("hash=%08X\n", hash);
	bsw_putu32l(&bsw, hash);

	send_bytes(bsw.buf, bsw.top);
	//for (int i = 0; i < bsw.top; i++)
	//	printf("%02X ", buf[i]);
	//printf("\n");
	free(buf);
}

#include <pthread.h>

char* argv1;
int pkt_ok = 1; // -1 pending; 0 fail; 1 ok
pthread_mutex_t pkt_ok_mx;

void* send_file(void* arg) {
	FILE* file = fopen(argv1, "rb");
	if (!file) {
		//perror("Failed to open file");
		return NULL;
	}

	unsigned char* buffer = malloc(MAX_PAGE_SIZE);
	size_t bytes_read;

	//printf("Sending file: %s\n", argv1);

	while ((bytes_read = fread(buffer, 1, MAX_PAGE_SIZE, file)) > 0) {
		while (1) {
			bool is_break = 0;
			pthread_mutex_lock(&pkt_ok_mx);
			if (pkt_ok == 1) {
				Sleep(1000);
				send_test((char*)buffer, bytes_read);
				pkt_ok = -1;
				is_break = 1;
			}
			pthread_mutex_unlock(&pkt_ok_mx);
			if (is_break) { break; }
			Sleep(20);
		}
		//send_test((char*)buffer, bytes_read);
		//Sleep(1000);
	}

	fclose(file);
	printf("File sent.\n");
	return NULL;
}

void handler(uint8_t b) {
	static bool debug_byte = 0;
	//printf("%02X\n", b);
	if (debug_byte) {
		printf("%c", b);
		debug_byte = 0;
		return;
	}
	if (b == CLIENT_DEBUG_LOG) {
		debug_byte = 1;
		return;
	} else if (b == CLIENT_ACK) {
		pthread_mutex_lock(&pkt_ok_mx);
		pkt_ok = 1;
		pthread_mutex_unlock(&pkt_ok_mx);
		printf("Hash check successful.\n");
	} else if (b == CLIENT_NAK) {
		pthread_mutex_lock(&pkt_ok_mx);
		pkt_ok = 0;
		pthread_mutex_unlock(&pkt_ok_mx);
		printf("Error. Sending again.\n");
	}
}

void timer_test() {
	static uint32_t last_time = 0;
	if (nob_ms() > last_time + 1000) {
		last_time = nob_ms();
		//pthread_mutex_lock(&pkt_ok_mx);
		//printf("pkt_ok=%d\n", pkt_ok);
		//pthread_mutex_unlock(&pkt_ok_mx);
		//printf("time: %lu\n", nob_ms());
	}
}

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Usage: %s <file>\n", argv[0]);
		return 1;
	}

	if (uartcom_init())
		return 1;

	argv1 = argv[1];
	pthread_t sf_thr;
	int sf_res;
	pkt_ok = 1;
	pthread_create(&sf_thr, NULL, *send_file, NULL);

	unsigned char b;
	DWORD n;

	while (1) {
		ReadFile(clienth, &b, 1, &n, NULL);
		if (n > 0)
			handler(b);
		timer_test();
	}
	pthread_join(sf_thr, NULL);
	CloseHandle(clienth);
	return 0;
}
