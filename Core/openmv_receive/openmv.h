#ifndef __OPENMV_H__
#define __OPENMV_H__

#include "main.h"

//此处的UART4/5还未测试，不知功能是否正常


#define OPENMV_UART USART2
#define rx_buffer_num 50



void Openmv_Data_Process(void);


#endif 