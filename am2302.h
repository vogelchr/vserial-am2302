#ifndef AM2302_H
#define AM2302_H

#include <stdint.h>

enum am2302_result {
	am2302_ok,
	am2302_ongoing,
	am2302_crc,
	am2302_timeout
};

/* setup everything */
extern void am2302_init(void);

/* trigger a read on the am2302, returns to caller immediately */
extern void am2302_trigger_read(void);

/* Debugging aid: put the 5 bytes returned from sender into
 * *data, the order is "as received", e.g. data[0] is the first
 * received byte (MSB r.h.), data[4] the last (CRC), returns
 * the internal counter of bits yet-to-receive from device, should
 * always be 0 (!= 0 means conversion ongoing, or mismatch of recvd bytes)
 */
extern unsigned char am2302_get_raw_data(unsigned char *data);

/* after a max. of about 6ms after issuing a request for conversion
 * using am3202_trigger_read(), you can call am3202_get_result().
 *
 * Return value indicates status of the measurement:
 *
 *   am2302_ok      : measurement ok, temp and rh is stored in the
 *                    location pointed to by temp and rh.
 *                    LSB of temp is 0.1 deg Celsius
 *                    LSB of rh   is 0.1% r.h.
 *
 *   am2302_ongoing : measurement has not finished
 *   am2302_crc     : measurement finished, but checksum is bad
 *   am2302_timeout : there was a timeout waiting for data from the sensor
 *                    No data is stored to *temp and *rh for the latter
 *                    three cases.
 */

extern enum am2302_result am2302_get_result(int16_t *temp, uint16_t *rh);

#endif
