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

void flash_select_protocol(uint8_t allowed_protocols)
{
	(void)allowed_protocols;
}

void flash_spiop(uint32_t sbytes, uint32_t rbytes)
{
	while (sbytes--) RECEIVE();
	SEND(S_ACK);
	while (rbytes--) SEND(0xFF);
}

uint32_t spi_set_speed(uint32_t hz)
{
	return hz;
}

void flash_set_safe(void)
{

}

