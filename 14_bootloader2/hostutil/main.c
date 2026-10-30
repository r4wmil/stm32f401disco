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

enum {
	STATE_PENDING = 0,
	STATE_WAITING_ACK
};

int state = STATE_PENDING;

void state_pending(uint8_t b) {
	if (b == 0x1E) {
		printf("Sending...\n");
		uint8_t text[] = "abcdefg0123";
		size_t text_len = strlen(text);
		uint32_t hash = crc_32(text, text_len);
		printf("hash=%08X\n", hash);

		uint8_t buf[1024];
		bswriter_t bsw = {.buf=buf,.top=0};
		bsw_putbuf(&bsw, (uint8_t[])PACKET_KEY, 4);
		bsw_putu32l(&bsw, text_len);
		bsw_putu32l(&bsw, hash);
		bsw_putbuf(&bsw, text, text_len);

		send_bytes(bsw.buf, bsw.top);
		for (int i = 0; i < bsw.top; i++)
			printf("%02X ", buf[i]);
		printf("\n");
		state = STATE_WAITING_ACK;
	}
}

void handler(uint8_t b) {
	//printf("%c", b);
	printf("%02X\n", b);
	switch (state) {
	case STATE_PENDING:
		state_pending(b);
		break;
	case STATE_WAITING_ACK:
		if (b == CLIENT_SUCCESS) { printf("Hash check successful.\n"); }
		if (b == CLIENT_ERROR) { printf("Error. Sending again.\n"); }
		break;
	}
}

int main(int argc, char** argv) {

	char crc_test[] = "testTest";
	printf("crc_test_hash=%08X\n", crc_32(crc_test, strlen(crc_test)));

	if (uartcom_init())
		return 1;

	unsigned char b;
	DWORD n;

	state = STATE_PENDING;
	printf("Handshaking...\n");
	assert(send_byte(HOST_SYN));
	while (1) {
		ReadFile(clienth, &b, 1, &n, NULL);
		if (n == 0) continue;
		handler(b);
		//if (b) { printf("%02X\n", b), fflush(stdout); }
	}

	CloseHandle(clienth);
}
