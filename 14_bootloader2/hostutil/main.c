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
	printf("hash=%08X\n", hash);
	bsw_putu32l(&bsw, hash);

	send_bytes(bsw.buf, bsw.top);
	for (int i = 0; i < bsw.top; i++)
		printf("%02X ", buf[i]);
	printf("\n");
	free(buf);
}

void handler(uint8_t b) {
	//printf("%02X\n", b);
	//printf("%c", b);
	if (b == CLIENT_ACK) { printf("Hash check successful.\n"); }
	if (b == CLIENT_NAK) { printf("Error. Sending again.\n"); }
}

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Usage: %s <file>\n", argv[0]);
		return 1;
	}

	if (uartcom_init())
		return 1;

	FILE* file = fopen(argv[1], "rb");
	if (!file) {
		perror("Failed to open file");
		return 1;
	}

	unsigned char* buffer = malloc(MAX_PAGE_SIZE);
	size_t bytes_read;

	printf("Sending file: %s\n", argv[1]);

	while ((bytes_read = fread(buffer, 1, MAX_PAGE_SIZE, file)) > 0) {
		send_test((char*)buffer, bytes_read);
		Sleep(1000);
	}

	fclose(file);
	printf("File sent.\n");

	unsigned char b;
	DWORD n;

	while (1) {
		ReadFile(clienth, &b, 1, &n, NULL);

		if (n > 0)
			handler(b);
	}

	CloseHandle(clienth);
	return 0;
}
