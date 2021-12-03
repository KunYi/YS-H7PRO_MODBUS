
#include "main.h"

#include "usart.h"
#define _REDIRECTION_UART_PRINTF  huart3

#if defined ( __CC_ARM )

/* ARMCC, MDK-ARM compiler*/
#include <stdio.h>

#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
};

FILE __stdout;
/**
 * @brief define _sys_exit() to avoid using semihosting mode
 * @param void
 * @return  void
 */
void _sys_exit(int x)
{
    x = x;
}

int fputc(int ch, FILE *f)
{
  #ifdef _REDIRECTION_UART_PRINTF
    HAL_UART_Transmit(&_REDIRECTION_UART_PRINTF, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  #else
    ITM_SendChar(ch);
  #endif

  return(ch);
}

int fgetc(FILE * f)
{
  uint8_t ch = 0;
	#ifdef _REDIRECTION_UART_PRINTF
  while(HAL_UART_Receive(&_REDIRECTION_UART_PRINTF, &ch, 1, HAL_MAX_DELAY)!=HAL_OK);
	#else
	ch = EIO;
	#endif
  return ch;
}

#elif defined ( __ICCARM__ )

/* IAR C compiler */

int fputc(int ch, FILE *f)
{
  #ifdef _REDIRECTION_UART_PRINTF
    HAL_UART_Transmit(&_REDIRECTION_UART_PRINTF, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  #else
    ITM_SendChar(ch);
  #endif

  return(ch);
}


int fgetc(FILE * f)
{
  uint8_t ch = 0;
	#ifdef _REDIRECTION_UART_PRINTF
  while(HAL_UART_Receive(&_REDIRECTION_UART_PRINTF, &ch, 1, HAL_MAX_DELAY)!=HAL_OK);
	#else
	ch = EIO;
	#endif
  return ch;
}

#elif defined ( __GNUC__ )

/* GNU GCC */

int _write(int fd, char * ptr, int len)
{
  #ifdef _REDIRECTION_UART_PRINTF
  HAL_UART_Transmit(&_REDIRECTION_UART_PRINTF, (uint8_t *) ptr, len, HAL_MAX_DELAY);
  #else
  for (int i = 0; i < len; i++) {
	  ITM_SendChar(*(ptr+i));
  }
  #endif
  return len;
}


int _read(int fd, char* ptr, int len) {
  HAL_StatusTypeDef hstatus;

  if (fd == STDIN_FILENO) {
		#ifdef _REDIRECTION_UART_PRINTF
    hstatus = HAL_UART_Receive(gHuart, (uint8_t *) ptr, 1, HAL_MAX_DELAY);
		#else
		return EIO; /* SWO not support read data */
		#endif
    if (hstatus == HAL_OK)
      return 1;
    else
      return EIO;
  }
  errno = EBADF;
  return -1;
}

#endif
