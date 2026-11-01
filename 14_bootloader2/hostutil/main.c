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

void send_test() {
	printf("Sending...\n");
	uint8_t text[] = "abcdefg0123";
	size_t text_len = strlen(text);

	uint8_t buf[1024];
	bswriter_t bsw = {.buf=buf,.top=0};
	bsw_putu8(&bsw, HOST_SOH);
	bsw_putu32l(&bsw, text_len);
	bsw_putbuf(&bsw, text, text_len);
	uint32_t hash = crc_32(bsw.buf + 1, bsw.top - 1);
	printf("hash=%08X\n", hash);
	bsw_putu32l(&bsw, hash);

	send_bytes(bsw.buf, bsw.top);
	for (int i = 0; i < bsw.top; i++)
		printf("%02X ", buf[i]);
	printf("\n");
}

void handler(uint8_t b) {
	printf("%02X\n", b);
	//printf("%c", b);
	if (b == CLIENT_ACK) { printf("Hash check successful.\n"); }
	if (b == CLIENT_NAK) { printf("Error. Sending again.\n"); }
}

int main(int argc, char** argv) {

	char crc_test[] = "testTest";
	printf("crc_test_hash32=%08X\n", crc_32(crc_test, strlen(crc_test)));
	printf("crc_test_hash16=%04X\n", crc_16(crc_test, strlen(crc_test)));

	if (uartcom_init())
		return 1;

	unsigned char b;
	DWORD n;

	send_test();
	while (1) {
		ReadFile(clienth, &b, 1, &n, NULL);
		if (n == 0) continue;
		handler(b);
		//if (b) { printf("%02X\n", b), fflush(stdout); }
	}

	CloseHandle(clienth);
}
