/*
 * Copyright (c) 2016, CESAR.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license. See the LICENSE file for details.
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <stdbool.h>

#include "spi.h"
#include "nrf24l01.h"
#include "nrf24l01_io.h"
#include "hal/time.h"

/*
 * First run the tool "./rpiecho -m server" to
 * enter in server mode and then, in another rpi,
 * run "./rpiecho -m client" to enter in client mode.
 */


#define MESSAGE "Gateway says hello!"
#define MESSAGE_SIZE sizeof(MESSAGE)

#define NRF24_PWR_0DBM    0b11
#define NRF24_CHANNEL_DEFAULT 10
#define DEV "/dev/spidev0.0"

char  buffer[32];
uint8_t err;
uint8_t broadcast_addr[5] = {0x8D, 0xD9, 0xBE, 0x96, 0xDE};
int8_t status;


int main(int argc, char *argv[])
{
	uint8_t spi_fd = io_setup(DEV);
	nrf24l01_init(DEV, NRF24_PWR_0DBM);
	nrf24l01_set_channel(spi_fd, NRF24_CHANNEL_DEFAULT);
	nrf24l01_set_standby(spi_fd);
	nrf24l01_open_pipe(spi_fd, 0, broadcast_addr, 0);
	printf("set ptx\n");
	nrf24l01_set_ptx(spi_fd, 0);

	while (1){
		printf("Status:");
		memcpy(buffer, MESSAGE, MESSAGE_SIZE);
		status = nrf24l01_ptx_data(spi_fd, buffer, MESSAGE_SIZE);
		nrf24l01_ptx_wait_datasent(spi_fd);
		delay_us(20000);
		printf("%d\n", status);
	}

	return 1;
}
