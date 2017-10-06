/*
 * HydraBus libfrser integration
 *
 * Copyright (C) 2017 Urja Rannikko <urjaman@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "main.h"
#include "frser.h"
#include "frser-flashapi.h"
#include "uart.h"
#include "bsp_spi.h"

/* frser I/O functions need to know where to put the data, and frser was not built for multi-instance... */
static t_hydra_console *frser_con = 0;

uint8_t frser_uart_recv(void)
{
	uint8_t c;

	if (chnRead(frser_con->sdu, &c, 1) == 0)
		c = 0xFF;

	return c;
}

void frser_uart_send(uint8_t c)
{
	chnWrite(frser_con->sdu, &c, 1);
}

void run_frser(t_hydra_console *con, int via_sync)
{
	/* N.B. This is racy, yet triggered by user action so... */
	if (frser_con) return;
	frser_con = con;

	frser_init();
	/* This is true if main.c detected a flashrom serprog sync sequence
	   so we pass those bytes to frser. */
	if (via_sync) {
		int i;
		for (i=0;i<8;i++) frser_operation(S_CMD_NOP); // 0x00
		frser_operation(S_CMD_SYNCNOP); // 0x10
	}
	while (1) {
		uint8_t op = frser_uart_recv();
		if ((op==' ')||(op==0xFF)) break;
		frser_operation(op);
	}
	frser_con = 0;
}


/* Why does this kind of a structure exist somewhere down in the console API? The duck does the console have
 * to do with what other interfaces the device happens to be using?. WTF. And why does this structure contain
 * huge communication buffers? WTF^2. */
static uint8_t spi_initialized = 0;
static mode_config_proto_t frser_spi_cfg = {
	.dev_num = BSP_DEV_SPI1,
	.dev_gpio_pull = MODE_CONFIG_DEV_GPIO_NOPULL,
	.dev_mode = DEV_SPI_MASTER,
	.dev_bit_lsb_msb = DEV_SPI_FIRSTBIT_MSB
};

void flash_select_protocol(uint8_t allowed_protocols)
{
	(void)allowed_protocols;
	/* This API is redundant. WTF^3. */
	bsp_spi_init(frser_spi_cfg.dev_num, &frser_spi_cfg);
	spi_initialized = 1;
}

void flash_spiop(uint32_t sbytes, uint32_t rbytes)
{
	/* This API is synchronous, so it'll never reach the requested SPI speed, but there
	 * will be gaps between the bytes on the SPI interface. With 168 Mhz the gaps
	 * might be very small, but anyways... */
	bsp_spi_select(frser_spi_cfg.dev_num);
	while (sbytes--) {
		uint8_t d = RECEIVE();
		bsp_spi_write_u8(frser_spi_cfg.dev_num, &d, 1);
	}
	SEND(S_ACK);
	while (rbytes--) {
		uint8_t d;
		bsp_spi_read_u8(frser_spi_cfg.dev_num, &d, 1);
		SEND(d);
	}
	bsp_spi_unselect(frser_spi_cfg.dev_num);
}

uint32_t spi_set_speed(uint32_t hz)
{
	uint32_t chz = 42000000UL; /* FIXME: get fPCLK from somewhere, divide that by 2 */
	uint8_t dsel = 0;
	do {
		if (hz >= chz) break;
		chz /= 2;
		dsel++;
	} while (dsel<7);
	/* The dev_speed parameter is inverted from what hardware uses (this division selector), so we
	 * just calculated what the hardware would use, and then we invert it to use this API... */
	frser_spi_cfg.dev_speed = 7 - dsel;
	/* If it was already initialized, re-initialize with the new speed. */
	if (spi_initialized) bsp_spi_init(frser_spi_cfg.dev_num, &frser_spi_cfg);
	return chz;
}

void flash_set_safe(void)
{
	bsp_spi_deinit(frser_spi_cfg.dev_num);
	spi_initialized = 0;
}

