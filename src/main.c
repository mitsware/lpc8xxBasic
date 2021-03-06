/***************************************************************************
	main.c
	LPC8xxシリーズ　クロック選択、ウォッチドッグタイマ機能サンプルプログラム

	マイコン: LPC810(NXP Semiconductors)

	LPCXpresso付属のクロック設定関数とウォッチドッグタイマ設定関数に不満を
	感じたので、その代替となる二つのライブラリを作った。
	本ファイルは、それらライブラリの動作確認用サンプルプログラムである。

	クロック選択や、ウォッチドッグタイマの設定は、core.h内の以下のシンボル
	の値を変更することで可能となる。

	・クロック関係
		MAIN_CLK_SEL	メインクロックの選択
		SYS_PLL_CLK		PLL入力クロックの選択
		SYS_PLL_RATE	PLL逓倍数
		SYS_CLK_DIV		システムクロックの分周値
		IRC_PDWON		内蔵オシレータ未使用時の選択

	・ウォッチドッグタイマ関連
		WWDT_MODE		WDT動作モード
		WWDT_FREQ		WDT用オシレータの周波数
		WWDT_DIV		WDT用オシレータ出力時の分周値
		WWDT_TIM_OUT	WDTタイムアウト時間
		WWDT_TIM_GUARD	WDTクリアガード時間
		WWDT_TIM_WARN	WDT警告割り込み発生時間

	これらシンボルについてはcore.h内で詳しく説明している。

	Sys_lib.cに動作クロックの設定を行う関数を含めている。
	以下にその一覧を示す。

		・Sys_iniLpc810
			従来のSystemInitを置き換えるもの。
		・Sys_getMainClk
			メインクロック値を取得する。
			USART/UARTの伝送速度の設定やIOCONの設定をするときに使用する。
			従来のライブラリでは得られなかったもの。
		・Sys_getSysClk
			システムクロック値を取得する。
			従来はSystemCoreClockを直接見ていたが、情報の隠蔽化を図るため
			関数化した。
			なお、従来通りSystemCoreClockを直接見ることも可能である。
		・SystemCoreClockUpdate
			互換性のため残してある。ただし、Sys_iniLpc810内でSystemCoreClock
			の初期設定も行っているので、本関数は、もはや何もしてない。

	Wdt_lib.cにウォッチドッグタイマ関連の関数を含めている。
	以下にその一覧を示す。

		・Wdt_ini
			WDTユニットを初期化するとともに(必要ならば)動作開始させる。
		・Wdt_getOscClk
			現在のWDT用オシレータの周波数を取得する。
		・Wdt_clr
			WDTのクリアを行う。
		・Wdt_procWarn
			WDT警告割り込み時の処理関数。
			警告割り込みを使用する場合は、本関数の名前で定義しておく必要が
			ある(サンプルとして本ファイル内で定義している)。
			使用しない場合は定義の必要はない。

	本サンプルプログラム(main.c)では、これらの関数の使用方法を示している。

	このサンプルプログラムで使用するマイコンはLPC810を想定しており、以下の
	ピン配置で使用する。

	           ┌─Ｕ─┐
	    PIO0_5 □１　８□ PIO0_0
	    PIO0_4 □２　７□ (Vss)
	     SWCLK □３　６□ (Vdd)
	     SWDIO □４　５□ CLKIN
	           └───┘

	pin3, 4はデバッガ接続用としている。
	そのほかのピンの機能は以下の通り。

	pin	機能	説明
	1	PIO0_5	GPIO出力ポートとして使用。各種情報を出力(LED_INFO)。
	2	PIO0_4	GPIO入力ポートとして使用。ロックアップテスト用(IN_PORT)。
	5	CLKIN	クロック選択でソースクロックをCLKINとした場合に使用する。
	8	PIO0_0	GPIO出力ポートとして使用。定期的に点滅出力(LED_SYSTICK)。

	・LED_INFO
		システムクロックの変化を見るため、一定のカウント毎にL/Hをトグル出力
		する。
		また、ウォッチドッグタイマ警告割り込みが発生した時にも、本ポートにH
		出力する。
		LEDをつなげば、クロックに応じて点滅の速度が変化するだろう。
		また、IN_PORTでロックアップ状態にしていた場合は、ウォッチドッグタイ
		マの満了も目視できる(ロックアップ状態で消灯～リセット時に一瞬点く)。

	・LED_SYSTICK
		SysTick割り込みを使って、割り込み発生毎にL/Hをトグル出力している。
		LEDをつなげば、クロック設定に関わらずに一定の周期で点滅表示するだろ
		う。

	・IN_PORT
		ウォッチドッグタイマのテスト用として、L入力があった時に擬似的なロッ
		クアップ状態を作る。
		本ポートは内部でプルアップされているので、オープンしておけば通常動
		作となる。
		ロックアップ状態にしたい場合は、本ポートをGNDにつなぐ。

	・CLKIN
		ソースクロックとして本ポートからの入力を使うことも可能である。
		これは、core.h内の定義に依存する。
		この場合は、ちゃんとしたクロックソースに接続しておく必要があるが、
		そうでない場合(内蔵オシレータを使う場合など)はオープンのままでも良
		い。

	変更履歴
	2014.06.07: mits: 新規作成
***************************************************************************/
#include	"core.h"
#include	"Sys_lib.h"		/* for Sys_* */
#include	"Wdt_lib.h"		/* for Wdt_* */

/***************************************************************************
	ローカル定義
***************************************************************************/

/*** 各GPIO毎の機能割り当て ***/
enum {
	LED_SYSTICK	= 0x1<<0,	/* P0_0をSysTickでの点滅表示で使う */
	IN_PORT		= 0x1<<4,	/* P0_4を入力ポートとして使う */
	LED_INFO	= 0x1<<5	/* P0_5を警告表示で使う */
};

/*** GPIO出力の指示 ***/
typedef enum Gpio_bit {
	GPIO_CLR	= 0,	/* ビットクリア */
	GPIO_SET	= 1,	/* ビットセット */
	GPIO_TOGGLE	= 2		/* ビットトグル */
} Gpio_bit;

/***************************************************************************
	ローカル関数
***************************************************************************/
static void setup(void);
static void SwitchMatrix_Init(void);
static void startSysTick(void);
static void iniPort(void);
static void setPort(uint32_t pat, Gpio_bit act);
static _Bool getGpioIsLow(void);

/***************************************************************************
	main

	[引数]	なし
	[戻値]	本関数からは戻らない
***************************************************************************/
int main(void)
{
	enum {
		LOAD_CNT = 0xffff	/* システムクロックの処理速度を測るためのカウンタ値 */
	};
	uint32_t load_cnt = LOAD_CNT;

	setup();

	for (;;) {
		/* 適当な回数数えたら点滅表示 */
		if (++load_cnt >= LOAD_CNT) {
			load_cnt = 0;
			setPort(LED_INFO, GPIO_TOGGLE);
		}
		/* GPIOでLow指定されたらロックアップ */
		if (getGpioIsLow()) {
			setPort(LED_INFO, GPIO_CLR);
			while (getGpioIsLow()) {
				;
			}
		}
		Wdt_clr();
	}
	return 0 ;
}

/***************************************************************************
	setup
	システム初期化関数

	[引数]	なし
	[戻値]	なし
***************************************************************************/
static void setup(void)
{
	SwitchMatrix_Init();	/* 本システムのピン配置を設定 */
	iniPort();				/* デバッグ用途もあるので最初にGPIOを初期化 */
	Sys_iniLpc810();		/* システム初期化(クロック選択とWDTの開始) */
	startSysTick();			/* SysTickタイマを開始 */
	Wdt_clr();
}

/***************************************************************************
	SwitchMatrix_Init
	本マイコンのピン配置の初期化
	※Switch Matrix Tool(以下)の吐き出したものをそのまま使用
		http://www.lpcware.com/content/nxpfile/nxp-switch-matrix-tool-lpc800

	[引数]	なし
	[戻値]	なし

	本ファームウェアでは以下のピン配置で使用する。

	           ┌─Ｕ─┐
	    PIO0_5 □１　８□ PIO0_0
	    PIO0_4 □２　７□ (Vss)
	     SWCLK □３　６□ (Vdd)
	     SWDIO □４　５□ CLKIN
	           └───┘
***************************************************************************/
static void SwitchMatrix_Init(void)
{
    /* Enable SWM clock */
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);

    /* Pin Assign 8 bit Configuration */
    /* none */

    /* Pin Assign 1 bit Configuration */
    /* SWCLK */
    /* SWDIO */
    /* CLKIN */
    LPC_SWM->PINENABLE0 = 0xffffff73UL; 
}

/***************************************************************************
	SysTick関連
***************************************************************************/

/***************************************************************************
	startSysTick
	SysTickタイマの開始

	[引数]	なし
	[戻値]	なし

	指定SYSTICK_MS(ms)毎にSysTickタイマを起動させるよう初期化する。
	本関数呼び出し後にSysTickタイマは動作開始する。

	計算途中の桁あふれを防ぐためにuint64_tを使用している。
	なお、コアライブラリ(SysTick_Config)内で割り込みレベルは3で初期化されて
	いる。
***************************************************************************/
static void startSysTick(void)
{
	enum {
		SYSTICK_MS = 250	/* ms; SysTick割り込みの起動間隔 */
	};
	uint64_t ticks = (uint64_t)Sys_getSysClk() * SYSTICK_MS / 1000;

	SysTick_Config(ticks);
}

/***************************************************************************
	SysTick_Handler
	システム組み込みのSysTickハンドラ

	[引数]	なし
	[戻値]	なし

	startSysTickで定義したSYSTICK_MS(ms)毎に起動する。
***************************************************************************/
void SysTick_Handler(void)
{
	setPort(LED_SYSTICK, GPIO_TOGGLE);
	/***
		一応念のためにコメントしておくが、システムクロックを一番遅い9.375kHz
		にした場合、1クロックが0.1msぐらいにしかならないため、上記のような
		単純なGPIO出力処理も10ms近くかかってしまう。

		つまり、その場合にSysTick割り込みの間隔(SYSTICK_MS)を10ms以下にして
		しまうと、割り込み間隔よりも処理時間の方がオーバーしてしまうため、
		本割り込み処理だけが連続して動き続けることになってしまう。
		結果として定常側(main)が一切動かなくなる。

		クロックを遅くする場合は、本ハンドラの処理を非常に軽くするか、
		SysTick割り込みの間隔(SYSTICK_MS)を長めに取るべきである。

		目安としては、割り込み間隔に対する割り込み処理時間の割合を10%以下に
		抑えるべきだろう。
	***/
}

/***************************************************************************
	GPIO制御
***************************************************************************/

/***************************************************************************
	iniPort
	本ファームウェアのGPIO関連の初期化

	[引数]	なし
	[戻値]	なし
***************************************************************************/
static void iniPort(void)
{
	LPC_SYSCON->SYSAHBCLKCTRL |= SYS_AHB_CLK_GPIO;	/* GPIOへクロック供給 */
	LPC_SYSCON->PRESETCTRL &= ~SYS_GPIO_RST_N;		/* GPIOをリセット～ */
	LPC_SYSCON->PRESETCTRL |= SYS_GPIO_RST_N;		/* リセット解除 */

	setPort(LED_SYSTICK | LED_INFO, GPIO_CLR);		/* ポートクリア */
	LPC_GPIO_PORT->DIR0 = LED_SYSTICK | LED_INFO;	/* 出力ポート化 */
}

/***************************************************************************
	setPort
	GPIOへの指定出力

	[引数]	pat	変化させたいGPIOのみ1をセット(b0:P0_0～b5:P0_5)
			act	以下のどれか
				GPIO_CLR	Low出力
				GPIO_SET	High出力
				GPIO_TOGGLE	反転出力
	[戻値]	なし

	指定のビットパターンでGPIOに対して出力を行う。
	CLR0, SET0, NOT0レジスタの機能(※)のおかげで、本関数はリエントラント性
	を保っている。
	そのため、割り込み側、定常(main)側、同時に本関数を使用できる。

	※UM10601 - Chapter 7:LPC800 GPIO port
***************************************************************************/
static void setPort(uint32_t pat, Gpio_bit act)
{
	switch (act) {
	case GPIO_CLR:
		LPC_GPIO_PORT->CLR0 = pat;
		break;
	case GPIO_SET:
		LPC_GPIO_PORT->SET0 = pat;
		break;
	case GPIO_TOGGLE:
		LPC_GPIO_PORT->NOT0 = pat;
		break;
	default:
		break;
	}
}

/***************************************************************************
	getGpioIsLow
	GPIO入力がLowかどうかの判断

	[引数]	なし
	[戻値]	Low(true), High(false)
***************************************************************************/
static _Bool getGpioIsLow(void)
{
	return ((LPC_GPIO_PORT->PIN0 & IN_PORT) == 0)? true: false;
}

/***************************************************************************
	ウォッチドッグタイマ関連
***************************************************************************/

/***************************************************************************
	Wdt_procWarn
	警告割り込み時の処理

	[引数]	なし
	[戻値]	なし

	本関数はウォッチドッグタイマ警告割り込み内から呼び出される。
***************************************************************************/
void Wdt_procWarn(void)
{
	setPort(LED_INFO, GPIO_SET);
}
