#ifndef __W25Q128_H__
#define __W25Q128_H__

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"

#define W25Q128_CS(x)  do { (x) ? HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_SET) \
                             : HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_RESET); \
                        } while (0)

#define FLASH_ManufactDevice                0x90
#define FLASH_WriteEnable                   0x06
#define FLASH_ReadStatusReg1                0x05
#define FLASH_ReadData                      0x03
#define FLASH_PageProgram                   0x02
#define FLASH_SectorErase                   0x20
#define FLASH_DummyByte                     0xFF

void w25q128_init(void);
uint16_t w25q128_read_id(void);
void w25q128_read_data(uint32_t address, uint8_t *data, uint32_t size);
void w25q128_write_page(uint32_t address, uint8_t *data, uint16_t size);
void w25q128_erase_sector(uint32_t address);

#endif
