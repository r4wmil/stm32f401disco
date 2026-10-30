#ifndef UARTCOM_H
#define UARTCOM_H

extern HANDLE clienth;

int send_byte(unsigned char b);
int send_bytes(uint8_t* buf, size_t len);

#endif /* UARTCOM_H */

#ifdef UARTCOM_IMPLEMENTATION

HANDLE clienth;

int send_byte(unsigned char b) {
	DWORD written;
	return WriteFile(clienth, &b, 1, &written, NULL) && written == 1;
}

int send_bytes(uint8_t* buf, size_t len) {
	DWORD written;
	return WriteFile(clienth, buf, len, &written, NULL) && written == len;
}

int uartcom_init() {
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
	return 0;
}

#endif /* UARTCOM_IMPLEMENTATION */
