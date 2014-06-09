/***************************************************************************
	lpc8xx_ctrl.h
	LPC8xx用各種定義

	マイコン: LPC8xx(NXP Semiconductors)

	ARM共通、及びLPC8xxマイコンのレジスタ関連の定義を含めた。
	コアライブラリ(CMSIS_CORE_LPC8xx)で定義されてなかったため、プログラム中
	にマジックナンバーを直接書くよりかは可読性の向上を図るため用意した。

	現版ではSYSCONとWWDT関連のものしか定義してない。
	必要に応じて追加していく予定。

	変更履歴
	2014.06.07: mits: 新規作成
***************************************************************************/
#ifndef	LPC8XX_CTRL_H
#define	LPC8XX_CTRL_H

/***************************************************************************
	ARM, CMSIS共通
***************************************************************************/

/* LPC8xxマイコンでの割り込み優先度(2ビットで表す) */
enum {
	PRI_TOP		= 0x00,	/* 最高位 */
	PRI_HIGH	= 0x01,
	PRI_MEDIUM	= 0x02,
	PRI_LOW		= 0x03	/* 最低位 */
};

/* コアライブラリSysTick_Config()に渡せるtick数の最大値 */
/* ※カウンタ自体は24ビットしかないが、SysTick_Config内で-1してから設定してる */
enum {
	SYSTICK_MAX = 0x1000000UL
};

/***************************************************************************
	SYSCON
***************************************************************************/

/* リセット制御レジスタ(LPC_SYSCON->PRESETCTRL) */
/* 0:リセット、1:リセット解除 */
enum {
	SYS_SPI0_RST_N		= 0x1<<0,	/* SPI0 reset control */
	SYS_SPI1_RST_N		= 0x1<<1,	/* SPI1 reset control */
	SYS_UARTFRG_RST_N	= 0x1<<2,	/* USART fractional baud rate generator(UARTFRG) reset control */
	SYS_UART0_RST_N		= 0x1<<3,	/* USART0 reset control */
	SYS_UART1_RST_N		= 0x1<<4,	/* USART1 reset control */
	SYS_UART2_RST_N		= 0x1<<5,	/* USART2 reset control */
	SYS_I2C_RST_N		= 0x1<<6,	/* I2C reset control */
	SYS_MRT_RST_N		= 0x1<<7,	/* Multi-rate timer (MRT) reset control */
	SYS_SCT_RST_N		= 0x1<<8,	/* SCT reset control */
	SYS_WKT_RST_N		= 0x1<<9,	/* Self wake-up timer (WKT) reset control */
	SYS_GPIO_RST_N		= 0x1<<10,	/* GPIO and GPIO pin interrupt reset control */
	SYS_FLASH_RST_N		= 0x1<<11,	/* Flash controller reset control */
	SYS_ACMP_RST_N		= 0x1<<12	/* Analog comparator reset control */
};

/* PLL状態レジスタ(LPC_SYSCON->SYSPLLSTAT) */
enum {
	SYS_PLL_STAT		= 0x1<<0	/* PLL起動状態(フェーズロック完了か否か) */
};
enum {
	SYS_PLL_NOT_LOCKED	= 0,	/* フェーズロック中 */
	SYS_PLL_LOCKED		= 1		/* フェーズロック完了 */
};

/* WDT用オシレータ制御レジスタ(LPC_SYSCON->WDTOSCCTRL) */
/* 選択可能な周波数の選択コード */
enum {
	WDTOSC_FREQ_DIS     = 0,	/* 禁止 */
	WDTOSC_FREQ_600KHZ  = 1,	/* 600kHz */
	WDTOSC_FREQ_1_05MHZ = 2,	/* 1.05MHz */
	WDTOSC_FREQ_1_40MHZ = 3,	/* 1.40MHz */
	WDTOSC_FREQ_1_75MHZ = 4,	/* 1.75MHz */
	WDTOSC_FREQ_2_10MHZ = 5,	/* 2.10MHz */
	WDTOSC_FREQ_2_40MHZ = 6,	/* 2.40MHz */
	WDTOSC_FREQ_2_70MHZ = 7,	/* 2.70MHz */
	WDTOSC_FREQ_3_00MHZ = 8,	/* 3.00MHz */
	WDTOSC_FREQ_3_25MHZ = 9,	/* 3.25MHz */
	WDTOSC_FREQ_3_50MHZ = 10,	/* 3.50MHz */
	WDTOSC_FREQ_3_75MHZ = 11,	/* 3.75MHz */
	WDTOSC_FREQ_4_00MHZ = 12,	/* 4.00MHz */
	WDTOSC_FREQ_4_20MHZ = 13,	/* 4.20MHz */
	WDTOSC_FREQ_4_40MHZ = 14,	/* 4.40MHz */
	WDTOSC_FREQ_4_60MHZ = 15	/* 4.60MHz */
};

/* システムリセット状態レジスタ(LPC_SYSCON->SYSRSTSTAT) */
/* 0:該当のリセット非検出、1:該当のリセット検出　※1書きで要因クリア */
enum {
	SYS_RST_POR		= 0x1<<0,	/* POR reset status */
	SYS_RST_EXTRST	= 0x1<<1,	/* External reset status . */
	SYS_RST_WDT		= 0x1<<2,	/* Status of the Watchdog reset */
	SYS_RST_BOD		= 0x1<<3,	/* Status of the Brown-out detect reset */
	SYS_RST_SYSRST	= 0x1<<4	/* Status of the s oftware sy stem reset */
};

/* PLL入力クロック選択レジスタ(LPC_SYSCON->SYSPLLCLKSEL) */
enum {
	SYS_PLL_CLK_SEL		= 0x3<<0,	/* 選択ビット */
};
enum {
	SYS_PLL_CLK_IRC		= 0x0,	/* 内蔵オシレータ(12MHz) */
	SYS_PLL_CLK_SYSOSC	= 0x1,	/* システムオシレータ　※LPC810には無し */
	SYS_PLL_CLK_CLKIN	= 0x3	/* CLKIN端子 */
};

/* PLL入力クロック更新レジスタ(LPC_SYSCON->SYSPLLCLKUEN) */
enum {
	SYS_PLL_CLK_UPDATE	= 0x1<<0	/* 0:変化なし、1:クロックソース更新完了 */
};

/* メインクロック選択レジスタ(LPC_SYSCON->MAINCLKSEL) */
enum {
	SYS_MAIN_CLK_SEL	= 0x3<<0	/* 選択ビット */
};
enum {
	SYS_MAIN_CLK_IRC	= 0x0,	/* 内蔵オシレータ(12MHz) */
	SYS_MAIN_CLK_PLLIN	= 0x1,	/* PLLへの入力クロック */
	SYS_MAIN_CLK_WDTOSC	= 0x2,	/* WDT用オシレータ */
	SYS_MAIN_CLK_PLLOUT	= 0x3	/* PLLからの出力クロック */
};

/* メインクロック更新レジスタ(LPC_SYSCON->MAINCLKUEN) */
enum {
	SYS_MAIN_CLK_UPDATE	= 0x1<<0	/* 0:変化なし、1:クロックソース更新完了 */
};

/* クロック供給レジスタ(LPC_SYSCON->SYSAHBCLKCTRL) */
/* 0:禁止、1:許可　※★のものはリセット直後から許可(=1)、その他は0 */
enum {
	SYS_AHB_CLK_SYS			= 0x1<<0,	/* ★Enables the clock for the AHB, the APB bridge, the Cortex-M0+ core clocks, */
										/* 　SYSCON, and the PMU. This bit is read only and always reads as 1. */
	SYS_AHB_CLK_ROM			= 0x1<<1,	/* ★Enables clock for ROM. */
	SYS_AHB_CLK_RAM			= 0x1<<2,	/* ★Enables clock for SRAM. */
	SYS_AHB_CLK_FLASHREG	= 0x1<<3,	/* ★Enables clock for flash register interface. */
	SYS_AHB_CLK_FLASH		= 0x1<<4,	/* ★Enables clock for flash. */
	SYS_AHB_CLK_I2C			= 0x1<<5,	/* Enables clock for I2C. */
	SYS_AHB_CLK_GPIO		= 0x1<<6,	/* Enables clock for GPIO port registers and GPIO pin interrupt registers. */
	SYS_AHB_CLK_SWM			= 0x1<<7,	/* Enables clock for switc h matrix. */
	SYS_AHB_CLK_SCT			= 0x1<<8,	/* Enables clock for state configurable timer. */
	SYS_AHB_CLK_WKT			= 0x1<<9,	/* Enables clock for self wake-up timer. */
	SYS_AHB_CLK_MRT			= 0x1<<10,	/* Enables clock for multi-rate timer. */
	SYS_AHB_CLK_SPI0		= 0x1<<11,	/* Enables clock for SPI0. */
	SYS_AHB_CLK_SPI1		= 0x1<<12,	/* Enables clock for SPI1.  */
	SYS_AHB_CLK_CRC			= 0x1<<13,	/* Enables clock for CRC. */
	SYS_AHB_CLK_UART0		= 0x1<<14,	/* Enables clock for USART0. */
	SYS_AHB_CLK_UART1		= 0x1<<15,	/* Enables clock for USART1. */
	SYS_AHB_CLK_UART2		= 0x1<<16,	/* Enables clock for USART2. */
	SYS_AHB_CLK_WWDT		= 0x1<<17,	/* Enables clock for WWDT. */
	SYS_AHB_CLK_IOCON		= 0x1<<18,	/* Enables clock for IOCON block. */
	SYS_AHB_CLK_ACMP		= 0x1<<19	/* Enables clock to analog comparator. */
};

/* CLKOUT端子クロック選択(LPC_SYSCON->CLKOUTSEL) */
/* ※CLKOUT_Setup()の引数としても使用可能 */
enum {
	SYS_CLKOUT_IRC		= 0x0,	/* 内蔵オシレータ(12MHz) */
	SYS_CLKOUT_SYSOSC	= 0x1,	/* システムオシレータ　※LPC810では不可 */
	SYS_CLKOUT_WDTOSC	= 0x2,	/* WDT用オシレータ */
	SYS_CLKOUT_MAINCLK	= 0x3	/* メインクロック */
};

/* パワーダウンレジスタ(LPC_SYSCON->PDRUNCFG) */
/* 0:パワーオン、1:パワーダウン　※★のものはリセット直後からパワーオン(=0)、その他は1 */
enum {
	SYS_IRCOUT_PD	= 0x1<<0,	/* ★IRC oscillator output power. */
	SYS_IRC_PD		= 0x1<<1,	/* ★IRC oscillator power down. */
	SYS_FLASH_PD	= 0x1<<2,	/* ★Flash power down. */
	SYS_BOD_PD		= 0x1<<3,	/* ★BOD power down. */
	SYS_SYSOSC_PD	= 0x1<<5,	/* Crystal oscillator power down. */
								/* 　After power-up, add a software delay of approximately 500 μs before using. */
	SYS_WDTOSC_PD	= 0x1<<6,	/* Watchdog oscillator power down. */
								/* 　Changing this bit to powered-down has no effect when the LOCK bit in the WWDT MOD register is set. */
								/* 　In this case, the watchdog oscillator is always running. */
	SYS_SYSPLL_PD	= 0x1<<7,	/* System PLL power down. */
	SYS_ACMP_PD		= 0x1<<15	/* Analog comparator power down . */
};

/***************************************************************************
	IOCON
***************************************************************************/

/* IOCONレジスタ(LPC_IOCON->PIO0_?) ※全レジスタ共通 */
enum {
	IOCON_MODE	= 0x3<<3,	/* Selects function mode (on-chip pull-up/pull-down resistor control). */
		IOCON_MODE_INACTIVE		= 0x0<<3,	/* Inactive (no pull-down/pull-up resistor enabled). */
		IOCON_MODE_PULL_DOWN	= 0x1<<3,	/* Pull-down resistor enabled. */
		IOCON_MODE_PULL_UP		= 0x2<<3,	/* Pull-up resistor enabled. */
		IOCON_MODE_REPEATER		= 0x3<<3,	/* Repeater mode. */
	IOCON_HYS	= 0x1<<5,	/* Hysteresis. */
		IOCON_HYS_OFF			= 0x0<<5,	/* Disable.  */
		IOCON_HYS_ON			= 0x1<<5,	/* Enable.  */
	IOCON_INV	= 0x1<<6,	/* Invert input. */
		IOCON_INV_OFF			= 0x0<<6,	/* Input not inverted (HIGH on pin reads as 1; LOW on pin reads as 0). */
		IOCON_INV_ON			= 0x1<<6,	/* Input inverted (HIGH on pin reads as 0, LOW on pin reads as 1). */
	IOCON_OD	= 0x1<<10,	/* Open-drain mode. Remark: This is not a true open-drain mode. */
		IOCON_OD_OFF			= 0x0<<10,	/* Disable. */
		IOCON_OD_ON				= 0x1<<10,	/* Open-drain mode enabled.  */
	IOCON_S_MODE	= 0x3<<11,	/* Digital filter sample mode. */
		IOCON_S_MODE_BYPASS		= 0x0<<11,	/* Bypass input filter. */
		IOCON_S_MODE_1CLOCK		= 0x1<<11,	/* 1 clock cycle. Input pulses shorter than one filter clock are rejected. */
		IOCON_S_MODE_2CLOCK		= 0x2<<11,	/* 2 clock cycles. Input pulses shorter than two filter clocks are rejected. */
		IOCON_S_MODE_3CLOCK		= 0x3<<11,	/* 3 clock cycles. Input pulses shorter than three filter clocks are rejected. */
	IOCON_CLK_DIV	= 0x7<<13,	/* Select peripheral clock divider for input filter sampling clock. Value 0x7 is reserved. */
		IOCON_CLK_DIV0			= 0x0<<13,	/* IOCONCLKDIV0. */
		IOCON_CLK_DIV1			= 0x1<<13,	/* IOCONCLKDIV1. */
		IOCON_CLK_DIV2			= 0x2<<13,	/* IOCONCLKDIV2. */
		IOCON_CLK_DIV3			= 0x3<<13,	/* IOCONCLKDIV3. */
		IOCON_CLK_DIV4			= 0x4<<13,	/* IOCONCLKDIV4. */
		IOCON_CLK_DIV5			= 0x5<<13,	/* IOCONCLKDIV5. */
		IOCON_CLK_DIV6			= 0x6<<13	/* IOCONCLKDIV6. */
};

/***************************************************************************
	Switch Matrix
***************************************************************************/

/* 固定ピンイネーブルレジスタ(LPC_SWM->PINENABLE0) */
/*--------------------------------------------------------------------------
	特定のピンにしか割り当てられない機能を割り当てられるようにするかどうか決める。
	実際に割り当てるかどうかは各ピン毎のピンアサインレジスタに設定する必要がある。
	0で“割り当て有効化”なので注意。リセット直後に既に割り当て有効となっているものを★で示す。
--------------------------------------------------------------------------*/
enum {
	SWM_ACMP_I1_DIS	= 0x1<<0,	/* 　0：8番pin(PIO0_0)にACMP_I1を割り当て有効化、1：無効 */
	SWM_ACMP_I2_DIS	= 0x1<<1,	/* 　0：5番pin(PIO0_1)にACMP_I2を割り当て有効化、1：無効 */
	SWM_SWCLK_DIS	= 0x1<<2,	/* ★0：3番pin(PIO0_3)にSWCLKを割り当て有効化、1：無効 */
	SWM_SWDIO_DIS	= 0x1<<3,	/* ★0：4番pin(PIO0_2)にSWDIOを割り当て有効化、1：無効 */
	SWM_XTALIN_DIS	= 0x1<<4,	/* 　0：PIO0_8にXTALINを割り当て有効化、1：無効　※LPC810では無効 */
	SWM_XTALOUT_DIS	= 0x1<<5,	/* 　0：PIO0_9にXTALOUTを割り当て有効化、1：無効　※LPC810では無効 */
	SWM_RESET_DIS	= 0x1<<6,	/* ★0：1番pin(PIO0_5)にRESETを割り当て有効化、1：無効 */
	SWM_CLKIN_DIS	= 0x1<<7,	/* 　0：5番pin(PIO0_1)にCLKINを割り当て有効化、1：無効 */
	SWM_VDDCMP_DIS	= 0x1<<8	/* 　0：PIO0_6にVDDCMPを割り当て有効化、1：無効　※LPC810では無効 */
};

/***************************************************************************
	ウィンドウウォッチドッグタイマ(WWDT)
***************************************************************************/

/* WWDTモードレジスタ(LPC_WWDT->MOD) */
/* ★のビットは一度1にセットすると書き換え不可 */
enum {
	WWDT_WDEN		= 0x1<<0,	/* ★WWDT有効化ビット、0:停止、1:開始 */
	WWDT_WDRESET	= 0x1<<1,	/* ★WWDTリセットビット、0:リセットしない、1:リセット */
	WWDT_WDTOF		= 0x1<<2,	/* 　WWDT満了フラグ、0:未満了、1:満了済み */
	WWDT_WDINT		= 0x1<<3	/* 　WWDT警告割り込みフラグ、0:未発生、1:発生済み */
	/* ※LOCK, WDPROTECTビットに関しては挙動がおかしいため使用しない */
	/* 　参考URL: http://mits-whisper.info/post/85408704581/lpc810-14 */
};

/* 各カウンタの最大値 */
enum {
	WWDT_CNT_MAX	= 0xFFFFFF,	/* WWDTカウンタ(LPC_WWDT->TC) */
	WWDT_WINDOW_MAX	= 0xFFFFFF,	/* WWDTウィンドウカウンタ(LPC_WWDT->WINDOW) */
	WWDT_WARN_MAX	= 0x3FF		/* WWDT警告割り込みカウンタ(LPC_WWDT->WARNINT) */
};

#endif	/* LPC8XX_CTRL_H */
