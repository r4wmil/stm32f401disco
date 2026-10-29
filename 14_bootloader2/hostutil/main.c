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

enum {
	STATE_PENDING = 0,
	STATE_WAITING_ACK
};

int state = STATE_PENDING;

HANDLE clienth;

int send_byte(unsigned char b) {
	DWORD written;
	return WriteFile(clienth, &b, 1, &written, NULL) && written == 1;
}

int send_bytes(uint8_t* buf, size_t len) {
	DWORD written;
	return WriteFile(clienth, buf, len, &written, NULL) && written == len;
}

void state_pending(uint8_t b) {
	if (b == 0x1E) {
		printf("Sending...\n");
		uint8_t text[] = "abcdefg0123";
		size_t text_len = strlen(text);
		uint32_t hash = crc_32(text, text_len);
		printf("hash=%08X\n", hash);

		uint8_t buf[1024];
		bswriter_t bsw = {.buf=buf,.top=0};
		bsw_putbuf(&bsw, (uint8_t[]){0x12,0xEF,0x34,0xCD}, 4);
		bsw_putu32l(&bsw, text_len);
		bsw_putu32l(&bsw, hash);
		bsw_putbuf(&bsw, text, text_len);

		send_bytes(bsw.buf, text_len + 12);
		for (int i = 0; i < text_len + 12; i++)
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
		if (b == 0xFA) { printf("Hash check successful.\n"); }
		if (b == 0xE4) { printf("Error. Sending again.\n"); }
		break;
	}
}

int main(int argc, char** argv) {

	char crc_test[] = "testTest";
	printf("crc_test_hash=%08X\n", crc_32(crc_test, strlen(crc_test)));

	char name[16];
	int avail = -1;
	for (int i = 1; i <= 256; i++) {
		snprintf(name, sizeof(name), "COM%d", i);
		clienth = CreateFileA(name, 0, 0, NULL,
			OPEN_EXISTING, 0, NULL);
		if (clienth == INVALID_HANDLE_VALUE) continue;
		avail = i;
		printf("%s\n", name);
		CloseHandle(clienth);
	}
	if (avail == -1) {
		printf("No available COM interface.");
		return 1;
	}

	snprintf(name, sizeof(name), "\\\\.\\COM%d", avail);
	printf("Using %s\n", name);
	clienth = CreateFileA(name, GENERIC_READ | GENERIC_WRITE, 0,
		NULL, OPEN_EXISTING, 0, NULL);

	if (clienth == INVALID_HANDLE_VALUE) return 1;

	DCB dcb = { .DCBlength = sizeof(dcb) };
	GetCommState(clienth, &dcb);
	dcb.BaudRate = CBR_9600;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	if (!SetCommState(clienth, &dcb)) return 1;

	COMMTIMEOUTS t = {0};
	t.ReadIntervalTimeout = MAXDWORD;
	SetCommTimeouts(clienth, &t);

	unsigned char b;
	DWORD n;

	state = STATE_PENDING;
	printf("Handshaking...\n");
	assert(send_byte(0xF1));
	while (1) {
		ReadFile(clienth, &b, 1, &n, NULL);
		if (n == 0) continue;
		handler(b);
		//if (b) { printf("%02X\n", b), fflush(stdout); }
	}

	CloseHandle(clienth);
}
