/*
 * dofptoa - do the grunge work to convert an fp number to ascii
 */
#include "config.h"
#include <stdio.h>
#include <string.h>

#include "ntp_fp.h"
#include "lib_strbuf.h"
#include "ntp_stdlib.h"

char *
dofptoa(
	u_fp fpv,
	bool neg,
	short ndec,
	bool msec
	)
{
	register uint8_t *cp, *cpend;
	register unsigned long val;
	register short dec;
	uint8_t cbuf[12];
	uint8_t *cpdec;
	char *buf;
	char *bp;

	/*
	 * Get a string buffer before starting
	 */
	LIB_GETBUF(buf);

	/*
	 * Zero out the buffer
	 */
	ZERO(cbuf);

	/*
	 * Set the pointers to point at the first
	 * decimal place.  Get a local copy of the value.
	 */
	cp = cpend = &cbuf[5];
	val = fpv;

	/*
	 * If we have to, decode the integral part
	 */
	if (!(val & 0xffff0000))
	    cp--;
	else {
		register u_short sv = (u_short)(val >> 16);
		register u_short tmp;
		register u_short ten = 10;

		do {
			tmp = sv;
			sv = (u_short) (sv/ten);
			*(--cp) = (uint8_t)(tmp - ((sv<<3) + (sv<<1)));
		} while (sv != 0);
	}

	/*
	 * Figure out how much of the fraction to do
	 */
	if (msec) {
		dec = (short)(ndec + 3);
		if (dec < 3)
		    dec = 3;
		cpdec = &cbuf[8];
	} else {
		dec = ndec;
		cpdec = cpend;
	}

	if (dec > 6)
	    dec = 6;
	
	if (dec > 0) {
		do {
			val &= 0xffff;
			val = (val << 3) + (val << 1);
			*cpend++ = (uint8_t)(val >> 16);
		} while (--dec > 0);
	}

	if (val & 0x8000) {
		register uint8_t *tp;
		/*
		 * Round it. Ick.
		 */
		tp = cpend;
		*(--tp) += 1;
		while (*tp >= 10) {
			*tp = 0;
			*(--tp) += 1;
		}
	}

	/*
	 * Remove leading zeroes if necessary
	 */
	while (cp < (cpdec -1) && *cp == 0)
	    cp++;
	
	/*
	 * Copy it into the buffer, asciizing as we go.
	 */
	bp = buf;
	if (neg)
	    *bp++ = '-';
	
	while (cp < cpend) {
		if (cp == cpdec)
		    *bp++ = '.';
		*bp++ = (char)(*cp++ + '0');
	}
	*bp = '\0';
	return buf;
}
