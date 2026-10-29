#ifndef BINSER_H

void bs_putu32l(uint8_t* p, uint32_t v);
uint32_t bs_getu32l(const uint8_t* p);

typedef struct bswriter_t {
	uint8_t* buf;
	size_t top;
} bswriter_t;

void bsw_putu32l(bswriter_t* bsw, uint32_t v);
void bsw_putbuf(bswriter_t* bsw, uint8_t* buf, size_t len);

#endif /* BINSER_H */

#ifdef BINSER_IMPLEMENTATION

void bs_putu32l(uint8_t* p, uint32_t v) {
	p[0] = (v & 0x000000FF) >>  0;
	p[1] = (v & 0x0000FF00) >>  8;
	p[2] = (v & 0x00FF0000) >> 16;
	p[3] = (v & 0xFF000000) >> 24;
}

uint32_t bs_getu32l(const uint8_t* p) {
	return
		  (uint32_t)p[0] <<  0
		| (uint32_t)p[1] <<  8
		| (uint32_t)p[2] << 16
		| (uint32_t)p[3] << 24;
}

void bsw_putu32l(bswriter_t* bsw, uint32_t v) {
	bs_putu32l(bsw->buf + bsw->top, v);
	bsw->top += 4;
}

void bsw_putbuf(bswriter_t* bsw, uint8_t* buf, size_t len) {
	memcpy(bsw->buf + bsw->top, buf, len);
	bsw->top += len;
}

#endif /* BINSER_IMPLEMENTATION */
