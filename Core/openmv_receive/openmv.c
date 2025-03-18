#include "openmv.h"
#include "stm32f4xx_hal.h"  // 确保包含HAL库头文件
#include <string.h>         // 包含memset函数头文件
#include <ctype.h>          // 包含isalpha函数头文件
#include <stdlib.h>         // 包含strtof函数头文件
#include <stdio.h>          // 包含printf函数头文件
#include "usart.h"

/*
1.双缓冲区
2.enum状态机
3.解析函数放在主函数
*/

enum Receive_State
{
    FIND_HEADER,
    RECEIVE_DATA,
    DATA_READY,
}UartRxState = FIND_HEADER;

float values[6];

uint8_t rx_buffer[rx_buffer_num];
volatile uint8_t rx_index = 0;
uint8_t parse_buffer[rx_buffer_num];

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    // 判断是否是期望的串口
    if (huart->Instance == OPENMV_UART)
    {
        // 定义一个静态的接收缓冲区
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);	
			switch(UartRxState)
			{
				case FIND_HEADER:
						if(rx_buffer[rx_index] == '#')
						{
								UartRxState = RECEIVE_DATA;
								rx_index +=1;
							
						}
						break;
				case RECEIVE_DATA:
						if (rx_buffer[rx_index] == ';')
						{
								UartRxState = DATA_READY;
								
						}
						else
						{
								rx_index++;
								if (rx_index > sizeof(rx_buffer))
								{
										rx_index = 0;
										return;
								}
						}
						break;
				case DATA_READY:
						break;

			
			}
    }
		HAL_UART_Receive_IT(&huart2, rx_buffer + rx_index, 1);
}

void Openmv_Data_Process(void)
{
    // 数据解析
    if (UartRxState == DATA_READY)
    {
        uint8_t parse_len = rx_index+1;
        memcpy(parse_buffer, rx_buffer, parse_len);
        
        char* ptr = (char*)parse_buffer;
        char* end;
        int i = 0;

        // 跳过开头的 '#'
        char* temp_ptr = strchr(ptr, '#');
        if (temp_ptr != NULL)
        {
            ptr = temp_ptr + 1;
        }
        else
        {
            // 未找到 '#' 情况下的处理
            rx_index = 0;
            memset(rx_buffer, 0, sizeof(rx_buffer));
            memset(parse_buffer, 0, sizeof(parse_buffer));
            HAL_UART_Receive_IT(&huart2, rx_buffer + rx_index, 1);  // 确保再次启用接收中断

        }

        if(parse_buffer[parse_len - 1]!= ';')
        {
            return;
        }

        // 解析浮点数
        while (*ptr && i < 6)
        {
            while (isalpha(*ptr)) ptr++;  // 跳过字母

            values[i] = strtof(ptr, &end);
            if (ptr == end) break;

            ptr = end;
            i++;

            // 跳过逗号
            while (*ptr == ',') ptr++;
        }

        // 处理解析后的数据
        for (int j = 0; j < i; j++)
        {
            printf("Value %c: %f\n", 'A' + j, values[j]);
        }

        // 重置状态
        rx_index = 0;
        memset(rx_buffer, 0, sizeof(rx_buffer));
				
				UartRxState = FIND_HEADER;
    }
	
		
}
