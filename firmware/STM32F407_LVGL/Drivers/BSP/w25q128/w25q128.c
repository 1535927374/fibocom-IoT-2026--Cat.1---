#include "w25q128.h"

SPI_HandleTypeDef spi_handle = {0};

void w25q128_spi_init(void)
{
    spi_handle.Instance               = SPI3;
    spi_handle.Init.Mode              = SPI_MODE_MASTER;
    spi_handle.Init.Direction         = SPI_DIRECTION_2LINES;
    spi_handle.Init.DataSize          = SPI_DATASIZE_8BIT;
    spi_handle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    spi_handle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    spi_handle.Init.NSS               = SPI_NSS_SOFT;
    spi_handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    spi_handle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    spi_handle.Init.TIMode            = SPI_TIMODE_DISABLE;
    spi_handle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    spi_handle.Init.CRCPolynomial     = 7;

    HAL_SPI_Init(&spi_handle);
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI3)
    {
        GPIO_InitTypeDef gpio_initstruct;

        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOF_CLK_ENABLE();
        __HAL_RCC_SPI3_CLK_ENABLE();

        gpio_initstruct.Pin       = GPIO_PIN_3;
        gpio_initstruct.Mode      = GPIO_MODE_OUTPUT_PP;
        gpio_initstruct.Pull      = GPIO_PULLUP;
        gpio_initstruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(GPIOF, &gpio_initstruct);
        HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_SET);

        gpio_initstruct.Pin       = GPIO_PIN_3;
        gpio_initstruct.Mode      = GPIO_MODE_AF_PP;
        gpio_initstruct.Pull      = GPIO_NOPULL;
        gpio_initstruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_initstruct.Alternate = GPIO_AF6_SPI3;
        HAL_GPIO_Init(GPIOB, &gpio_initstruct);

        gpio_initstruct.Pin       = GPIO_PIN_11 | GPIO_PIN_12;
        gpio_initstruct.Mode      = GPIO_MODE_AF_PP;
        gpio_initstruct.Pull      = GPIO_NOPULL;
        gpio_initstruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio_initstruct.Alternate = GPIO_AF6_SPI3;
        HAL_GPIO_Init(GPIOC, &gpio_initstruct);
    }
}

uint8_t w25q128_spi_swap_byte(uint8_t data)
{
    uint8_t recv_data = 0;
    HAL_SPI_TransmitReceive(&spi_handle, &data, &recv_data, 1, 1000);
    return recv_data;
}

void w25q128_init(void)
{
    w25q128_spi_init();
}

uint16_t w25q128_read_id(void)
{
    uint16_t device_id = 0;

    W25Q128_CS(0);
    w25q128_spi_swap_byte(FLASH_ManufactDevice);
    w25q128_spi_swap_byte(0x00);
    w25q128_spi_swap_byte(0x00);
    w25q128_spi_swap_byte(0x00);

    device_id  = (uint16_t)(w25q128_spi_swap_byte(FLASH_DummyByte) << 8);
    device_id |= w25q128_spi_swap_byte(FLASH_DummyByte);

    W25Q128_CS(1);
    return device_id;
}

void w25q128_write_enable(void)
{
    W25Q128_CS(0);
    w25q128_spi_swap_byte(FLASH_WriteEnable);
    W25Q128_CS(1);
}

uint8_t w25q128_read_sr1(void)
{
    uint8_t recv_data = 0;

    W25Q128_CS(0);
    w25q128_spi_swap_byte(FLASH_ReadStatusReg1);
    recv_data = w25q128_spi_swap_byte(FLASH_DummyByte);
    W25Q128_CS(1);
    return recv_data;
}

void w25q128_wait_busy(void)
{
    while ((w25q128_read_sr1() & 0x01) == 0x01) { }
}

void w25q128_send_address(uint32_t address)
{
    w25q128_spi_swap_byte((uint8_t)(address >> 16));
    w25q128_spi_swap_byte((uint8_t)(address >> 8));
    w25q128_spi_swap_byte((uint8_t)address);
}

void w25q128_read_data(uint32_t address, uint8_t *data, uint32_t size)
{
    uint32_t i = 0;

    W25Q128_CS(0);
    w25q128_spi_swap_byte(FLASH_ReadData);
    w25q128_send_address(address);

    for (i = 0; i < size; i++)
        data[i] = w25q128_spi_swap_byte(FLASH_DummyByte);

    W25Q128_CS(1);
}

void w25q128_write_page(uint32_t address, uint8_t *data, uint16_t size)
{
    uint16_t i = 0;

    w25q128_write_enable();

    W25Q128_CS(0);
    w25q128_spi_swap_byte(FLASH_PageProgram);
    w25q128_send_address(address);
    for (i = 0; i < size; i++)
        w25q128_spi_swap_byte(data[i]);
    W25Q128_CS(1);

    w25q128_wait_busy();
}

void w25q128_erase_sector(uint32_t address)
{
    w25q128_write_enable();
    w25q128_wait_busy();

    W25Q128_CS(0);
    w25q128_spi_swap_byte(FLASH_SectorErase);
    w25q128_send_address(address);
    W25Q128_CS(1);

    w25q128_wait_busy();
}
