/* ============================================================
 * 基础版实现片段（HAL_Delay 阻塞延时）
 * 说明：本文件仅作参考留档，不参与编译。
 *      完整基础版就是把下面两段贴进 Core/Src/main.c 对应的
 *      USER CODE 区域（并删掉非阻塞版的相关代码）。
 * ============================================================ */

/* ---------- Core/Src/main.c  USER CODE BEGIN 0 ---------- */
#define FLOW_DELAY_MS       500U

#define LED_ON(port, pin)   HAL_GPIO_WritePin((port), (pin), GPIO_PIN_SET)
#define LED_OFF(port, pin)  HAL_GPIO_WritePin((port), (pin), GPIO_PIN_RESET)

/* ---------- Core/Src/main.c  USER CODE BEGIN 3（while 循环内） ---------- */
#if 0
    static uint8_t step = 0U;

    /* 三灯全灭 */
    LED_OFF(LED0_GPIO_Port, LED0_Pin);
    LED_OFF(LED1_GPIO_Port, LED1_Pin);
    LED_OFF(LED2_GPIO_Port, LED2_Pin);

    /* 只点亮当前这一颗，形成"流水"效果 */
    switch (step)
    {
      case 0U:  LED_ON(LED0_GPIO_Port, LED0_Pin); break;
      case 1U:  LED_ON(LED1_GPIO_Port, LED1_Pin); break;
      default:  LED_ON(LED2_GPIO_Port, LED2_Pin); break;
    }

    step = (uint8_t)((step + 1U) % 3U);

    /* 阻塞延时 500ms */
    HAL_Delay(FLOW_DELAY_MS);
#endif
