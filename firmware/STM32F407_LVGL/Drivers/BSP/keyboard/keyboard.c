#include "keyboard.h"
#include "delay.h"

/* Rows: PD0 R1, PD1 R2, PD3 R3, PD4 R4 | Cols: PD5 C1, PD6 C2, PD7 C3 */
/* Scan: one row OUT low, other rows OUT high, read columns (pull-up, key pulls low). */

#define KEY_MIN_INTERVAL_MS  100U
#define KEY_STABLE_MS        22U

static uint32_t s_last_emit_tick = 0;

static const uint8_t KB_KEYS[4][3] = {
    { '1', '2', '3' },
    { '4', '5', '6' },
    { '7', '8', '9' },
    { '*', '0', '#' },
};

static const uint8_t kb_row_from_pin_idx[4] = { 0, 1, 2, 3 };

static const uint16_t KB_ROW_PINS[4] = {
    GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_3, GPIO_PIN_4
};

static void kb_cols_input_pullup(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &g);
}

static void kb_cols_idle(void)
{
    kb_cols_input_pullup();
}

static void kb_row_output_low(uint16_t pin)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = pin;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &g);
    HAL_GPIO_WritePin(GPIOD, pin, GPIO_PIN_RESET);
}

static void kb_row_output_high(uint16_t pin)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = pin;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &g);
    HAL_GPIO_WritePin(GPIOD, pin, GPIO_PIN_SET);
}

static void kb_rows_cols_restore_idle(void)
{
    GPIO_InitTypeDef g = {0};
    g.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &g);
    kb_cols_idle();
}

static void kb_spin_wait(void)
{
    volatile uint32_t n;
    for (n = 0; n < 3500u; n++)
    {
        __NOP();
    }
}

static uint8_t kb_col_mask_to_index(uint8_t cm)
{
    if (cm == 1u)
    {
        return 0;
    }
    if (cm == 2u)
    {
        return 1;
    }
    if (cm == 4u)
    {
        return 2;
    }
    return 0xFF;
}

static uint8_t kb_strobe_sample(void)
{
    uint8_t found = 0;
    uint8_t fr = 0;
    uint8_t fc = 0;
    uint8_t r;
    uint8_t j;

    for (r = 0; r < 4u; r++)
    {
        for (j = 0; j < 4u; j++)
        {
            if (j == r)
            {
                kb_row_output_low(KB_ROW_PINS[j]);
            }
            else
            {
                kb_row_output_high(KB_ROW_PINS[j]);
            }
        }

        kb_cols_input_pullup();
        kb_spin_wait();

        {
            uint8_t cm = 0;
            if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_5) == GPIO_PIN_RESET)
            {
                cm |= 1u;
            }
            if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == GPIO_PIN_RESET)
            {
                cm |= 2u;
            }
            if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_7) == GPIO_PIN_RESET)
            {
                cm |= 4u;
            }

            if (cm == 0u)
            {
                continue;
            }
            if ((cm & (cm - 1u)) != 0u)
            {
                kb_rows_cols_restore_idle();
                return 0;
            }

            {
                uint8_t cidx = kb_col_mask_to_index(cm);
                if (cidx > 2u)
                {
                    kb_rows_cols_restore_idle();
                    return 0;
                }
                if (found != 0u)
                {
                    kb_rows_cols_restore_idle();
                    return 0;
                }
                found = 1u;
                fr = r;
                fc = cidx;
            }
        }
    }

    kb_rows_cols_restore_idle();

    if (found == 0u)
    {
        return 0;
    }

    {
        uint8_t log_row = kb_row_from_pin_idx[fr];
        if (log_row > 3u)
        {
            return 0;
        }
        return KB_KEYS[log_row][fc];
    }
}

void keyboard_init(void)
{
    GPIO_InitTypeDef gpio_initstruct;

    __HAL_RCC_GPIOD_CLK_ENABLE();

    HAL_NVIC_DisableIRQ(EXTI0_IRQn);
    HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    HAL_NVIC_DisableIRQ(EXTI3_IRQn);
    HAL_NVIC_DisableIRQ(EXTI4_IRQn);

    gpio_initstruct.Mode = GPIO_MODE_INPUT;
    gpio_initstruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 | GPIO_PIN_4;
    gpio_initstruct.Pull = GPIO_PULLUP;
    gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &gpio_initstruct);

    kb_cols_idle();
}

uint8_t keyboard_get_value(void)
{
    uint8_t  ch;
    uint32_t now;
    static uint8_t  s_need_release = 0;
    static uint8_t  s_pending_ch = 0;
    static uint32_t s_stable_since = 0;

    ch = kb_strobe_sample();
    now = HAL_GetTick();

    if (s_need_release != 0u)
    {
        if (ch == 0)
        {
            s_need_release = 0u;
            s_pending_ch = 0;
            s_stable_since = 0;
        }
        return 0;
    }

    if (ch == 0)
    {
        s_pending_ch = 0;
        s_stable_since = 0;
        return 0;
    }

    if (ch != s_pending_ch)
    {
        s_pending_ch = ch;
        s_stable_since = now;
        return 0;
    }

    if ((now - s_stable_since) < KEY_STABLE_MS)
    {
        return 0;
    }

    if (s_last_emit_tick != 0U && (now - s_last_emit_tick) < KEY_MIN_INTERVAL_MS)
    {
        return 0;
    }

    if (kb_strobe_sample() != ch)
    {
        s_pending_ch = 0;
        s_stable_since = 0;
        return 0;
    }

    s_last_emit_tick = now;
    s_need_release = 1u;
    s_pending_ch = 0;
    s_stable_since = 0;
    return ch;
}
