/*
 * Copyright (c) 2017, CESAR.
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

#include "spi_bus.h"
#include "nrf24l01.h"
#include "nrf24l01_io.h"
#include "hal/time.h"
#include <errno.h>

#define MESSAGE "This is a test message"
#define MESSAGE_SIZE sizeof(MESSAGE)

#define NRF24_ADDR_WIDTHS	5
#define PIPE				1 // 0 for broadcast, 1 to 5 for data
#define PIPE_MAX			6
#define NRF24_MTU			32
#define CH_BROADCAST		76
#define CH_RAW				22
#define MAX_RT				50
#define DEV					"/dev/spidev0.0"

static char *opt_mode = "server";
static bool aack, server; //auto-ack and server/client flags
static int8_t pipe, rx_len, tx_len, tx_status, tx_pipe, err;
static int32_t tx_stamp, attempt;
static uint8_t rx_buffer[NRF24_MTU], tx_buffer[NRF24_MTU];
static uint8_t count, spi_fd;
static uint8_t pipe_addr[PIPE_MAX][NRF24_ADDR_WIDTHS] = {
	{0x8D, 0xD9, 0xBE, 0x96, 0xDE},
	{1, 0xBE, 0xEF, 0xDE, 0x96},
	{2, 0xBE, 0xEF, 0xDE, 0x96},
	{3, 0xBE, 0xEF, 0xDE, 0x96},
	{4, 0xBE, 0xEF, 0xDE, 0x96},
	{5, 0xBE, 0xEF, 0xDE, 0x96}
};

struct s_msg {
	uint8_t count;
	char msg[NRF24_MTU-1];
} __attribute__ ((packed));

#define MSG_SIZE (sizeof(((struct s_msg *)NULL)->count) + MESSAGE_SIZE)

union {
	char buffer[NRF24_MTU];
	struct s_msg data;
} rx;

union {
	char buffer[NRF24_MTU];
	struct s_msg data;
} tx;


static GOptionEntry options[] = {
	{ "mode", 'm', 0, G_OPTION_ARG_STRING, &opt_mode,
	"mode", "Operation mode: server or client"},
	{ "ack", 'a', 0, G_OPTION_ARG_INT, &aack,
	"ack", "Connection channel: broadcast or data(auto-ack)"},
	{ NULL },
};

/*
 * First run the tool "./rpiecho -m server" to
 * enter in server mode and then, in another rpi,
 * run "./rpiecho -m client" to enter in client mode.
 */

static void setup_radio(uint8_t spi_fd, int8_t aack, bool server)
{
	/*Raw Setup*/
	if (aack) {
		nrf24l01_open_pipe(spi_fd, PIPE, pipe_addr[0]);
		nrf24l01_set_channel(spi_fd, CH_RAW, aack);
		if (server) {
			printf("Data Server Listening\n");
			for (pipe = 0; pipe < PIPE_MAX; ++pipe)
				nrf24l01_open_pipe(spi_fd, pipe, pipe_addr[pipe]);
			nrf24l01_set_prx(spi_fd);
		} else {
			printf("Data Client Transmitting\n");
		}

	/*Broadcast Setup*/
	} else {
		nrf24l01_open_pipe(spi_fd, 0, pipe_addr[0]);
		nrf24l01_set_channel(spi_fd, CH_BROADCAST, 0);
		if (server) {
			printf("Broadcast Server Listening\n");
			nrf24l01_set_prx(spi_fd);
		} else {
			printf("Broadcast Client Transmitting\n");
		}
	}
};

static int running_raw(bool server)
{


	while (1) {
		/*Server-side Code*/
		if (server) {
			if (tx_len != 0 && (hal_time_ms() - tx_stamp) > 5) {
				memcpy(rx.buffer, tx.buffer, tx_len);
				nrf24l01_set_ptx(spi_fd, tx_pipe);
				if (nrf24l01_ptx_data(spi_fd, rx.buffer, 
													tx_len) == 0) {
					tx_status = nrf24l01_ptx_wait_datasent(spi_fd);
					if (tx_status == 0) {
						printf("TX%d[%d]:", tx_pipe, tx_len);
						printf("(%d)", tx.data.count);
						printf("%s\n", tx.data.msg);
						tx_len = 0;
					}
				} else
					printf("** TX FIFO FULL **\n");
				nrf24l01_set_prx(spi_fd);
				tx_stamp = hal_time_ms();
			}

			pipe = nrf24l01_prx_pipe_available(spi_fd);

			if (pipe != NRF24_NO_PIPE) {
				rx_len = nrf24l01_prx_data(spi_fd, rx.buffer,
					NRF24_MTU);
				if (rx_len != 0) {
					if (pipe < PIPE_MAX) {
						printf("RX%d[%d]:", pipe, rx_len);
						printf("'(%d)", rx.data.count);
						printf("%s\n", rx.data.msg);
						memcpy(tx.buffer, rx.buffer, rx_len);
						tx_len = rx_len;
						tx_pipe = pipe;
						tx_stamp = hal_time_ms();
					} else {
						printf("Invalid RX[%d:%d]:'%s'\n", pipe,
												rx_len, rx.buffer);
					}
				}
			}
		/*Client-side Code*/
		} else {
			if ((hal_time_ms() - tx_stamp) > 11) {
				memcpy(tx.data.msg, MESSAGE, MESSAGE_SIZE);
				tx.data.count = count;
				nrf24l01_set_ptx(spi_fd, PIPE);
				if (nrf24l01_ptx_data(spi_fd, tx.buffer,
					MSG_SIZE) == 0) {
					tx_status = nrf24l01_ptx_wait_datasent(spi_fd);
					if (tx_status == 0) {
						printf("TX%d[%lu:%d]:(%d)%s\n",
							PIPE,
							MESSAGE_SIZE,
							attempt,
							count,
							MESSAGE);
						attempt = 0;
						++count;
					} else
						++attempt;
				} else
					printf("** TX FIFO FULL **\n");
				nrf24l01_set_prx(spi_fd);
				tx_stamp = hal_time_ms();
			}

			pipe = nrf24l01_prx_pipe_available(spi_fd);
			if (pipe != NRF24_NO_PIPE) {
				rx_len = nrf24l01_prx_data(spi_fd,
											rx.buffer, NRF24_MTU);
				if (rx_len != 0) {
					if (pipe == PIPE) {
						printf("RX%d[%d]:%c(%d)%s\n",
							pipe,
							rx_len,
							rx.data.count != (count-1)?'*':' ',
							rx.data.count,
							rx.data.msg);
					} else {
						printf("Invalid RX[%d:%d]:'%s'\n", pipe,
							rx_len, rx.buffer);
					}
				}
			}
		}
	}
	/*To-Do: Return possible errors*/
	return 0;
};


static int running_bdcast(bool server)
{
	while (1) {
		if (server) {
			/*Server-side Code*/
			if (rx_len != 0 && (hal_time_ms() - tx_stamp) > 3) {
				memcpy(tx_buffer, rx_buffer, rx_len);
				nrf24l01_set_ptx(spi_fd, 0);
				if (nrf24l01_ptx_data(spi_fd, tx_buffer,
					rx_len) == 0)
					nrf24l01_ptx_wait_datasent(spi_fd);
				nrf24l01_set_prx(spi_fd);
				tx_stamp = hal_time_ms();
			}

			if (nrf24l01_prx_pipe_available(spi_fd) == 0) {
				rx_len = nrf24l01_prx_data(spi_fd,
					rx_buffer, NRF24_MTU);
				if (rx_len != 0) {
					printf("RX[%d]:'%s'\n",
						rx_len, rx_buffer);
					memcpy(rx_buffer, MESSAGE,
						MESSAGE_SIZE-1);
				}
			}

		} else {
			/*Client-side Code*/
			if ((hal_time_ms() - tx_stamp) > 5) {
				memcpy(tx_buffer, MESSAGE, MESSAGE_SIZE);
				nrf24l01_set_ptx(spi_fd, 0);
				if (nrf24l01_ptx_data(spi_fd,
					tx_buffer, MESSAGE_SIZE) == 0)
					nrf24l01_ptx_wait_datasent(spi_fd);
				tx_stamp = hal_time_ms();
				nrf24l01_set_prx(spi_fd);
			}

			if (nrf24l01_prx_pipe_available(spi_fd) == 0)
				rx_len = nrf24l01_prx_data(spi_fd,
					rx_buffer, NRF24_MTU);
			if (rx_len != 0) {
				printf("TX[%d]:'%s'; RX[%d]:'%s'\n",
					(int)MESSAGE_SIZE, MESSAGE,
					rx_len, rx_buffer);
				printf("Broadcasting...\n");
			}
		}
	}
	/*To-Do: Return possible errors*/
	return 0;
};

int main(int argc, char *argv[])
{

	/*Options parsing*/
	GOptionContext *context;
	GError *gerr = NULL;

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	if (!g_option_context_parse(context, &argc, &argv, &gerr)) {
		printf("Invalid arguments: %s\n", gerr->message);
		g_error_free(gerr);
		g_option_context_free(context);
		return EXIT_FAILURE;
	}

	/*Reset config*/
	rx_len = 0;
	tx_len = 0;
	tx_stamp = 0;
	attempt = 0;
	count = 0;

	/*Initialize Radio*/
	spi_fd = io_setup(DEV);
	nrf24l01_init(DEV, NRF24_PWR_0DBM);
	nrf24l01_set_standby(spi_fd);

	if (strcmp(opt_mode, "server") == 0)
		server = true; /*Server Mode*/
	else
		server = false; /*Client Mode*/

	 setup_radio(spi_fd, aack, server);

	if (aack) {
		/*Data-Channel*/
		err = running_raw(server);
	} else {
		/*Broadcast-Channel*/
		err = running_bdcast(server);
	}
	/*To-Do: Check for possible errors*/
	return err;
}
