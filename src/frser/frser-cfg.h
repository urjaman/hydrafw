#pragma once

//#define FRSER_FEAT_PARALLEL
//#define FRSER_FEAT_LPCFWH
#define FRSER_FEAT_SPI

/* Debug feature, if you want to use get_last_op to know the last frser operation. */
//#define FRSER_FEAT_LAST_OP
/* Safety feature, calls set_uart_timeout with a jmp_buf to longjmp to in case of timeout. */
//#define FRSER_FEAT_UART_TIMEOUT
/* If your system is capable of autodetecting the actual attached chip bustype. */
//#define FRSER_FEAT_DYNPROTO
/* If your system is capable of turning on/off chip drivers. */
#define FRSER_FEAT_PIN_STATE
/* If you have a debug console you'd want frser to call upon space bar. */
//#define FRSER_FEAT_DBG_CONSOLE

/* Name provided to flashrom to identify what the thing is. Max 16 bytes */
#define FRSER_NAME "HydraBus SPI"

/* Attached address lines, only if FRSER_FEAT_PARALLEL */
//#define FRSER_PARALLEL_BITS 19

/* Ability to set SPI frequency (only if SPI). */
#define FRSER_FEAT_SPISPEED

/* If you want to override the opbuf size completely, define this: */
#define FRSER_OPBUF_LEN 4096

/* If you want to override the read-n max len (default based on BAUD or 64k), define this: */
#define FRSER_READ_N_MAX (256*1024)

