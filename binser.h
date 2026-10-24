#ifndef BINSER_H

void bs_putu32l(uint8_t* p, uint32_t v);
uint32_t bs_getu32(const uint8_t* p);

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

#endif /* BINSER_IMPLEMENTATION */
