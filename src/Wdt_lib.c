/***************************************************************************
	Wdt_lib.c
	ウォッチドッグタイマライブラリ

	使用方法: #include "Wdt_lib.h"

	マイコン: LPC8xx(NXP Semiconductors)

	LPC800シリーズのウォッチドッグタイマ(WDT)を制御するAPI群。
	・Wdt_ini
		WDTユニットを初期化するとともに(必要ならば)動作開始させる。
	・Wdt_getOscClk
		現在のWDT用オシレータの周波数を取得する。
	・Wdt_clr
		WDTのクリアを行う。
	・Wdt_procWarn
		WDT警告割り込み時の処理関数。
		本関数は外部で定義しておく必要がある。
		警告割り込みを使用しない場合は定義の必要はない。

	変更履歴
	2014.06.07: mits: 新規作成
***************************************************************************/
#include	"core.h"
#include	"Wdt_lib.h"

/***************************************************************************
	ローカル変数
***************************************************************************/
static uint32_t	Wdt_freq;	/* WDTオシレータソースクロック(単位はHz) */
static uint32_t	Wdt_div;	/* 分周値(2～64の偶数) */

/***************************************************************************
	ローカル関数
***************************************************************************/
static uint32_t	Wdt_calcOscCtrl(uint32_t freq, uint32_t div);
static uint32_t	Wdt_getMs(uint32_t ms, uint32_t max);

/***************************************************************************
	Wdt_procWarn
	警告割り込み時の処理

	[引数]	なし
	[戻値]	なし

	本関数はweak定義しているので、必要ならば外部で用意しておく。
	本関数は置換されることを見越した空のダミー関数である。
***************************************************************************/
__attribute__ ((weak)) void Wdt_procWarn(void);
void Wdt_procWarn(void)
{
	/* 何もしない */
}

/***************************************************************************
	WDT_IRQHandler
	システム組み込みのWDT警告割り込みハンドラ

	[引数]	なし
	[戻値]	なし
***************************************************************************/
void WDT_IRQHandler(void)
{
	Wdt_procWarn();

	/***
		割り込み要因のクリア方法がユーザーズマニュアルの記述と異なっている。
		以下のページを参照して頂きたい。
			http://mits-whisper.info/post/85408704581/lpc810-14
	***/
	LPC_WWDT->MOD |= WWDT_WDINT;
	LPC_WWDT->MOD &= ~WWDT_WDTOF;
}

/***************************************************************************
	Wdt_ini
	WDTユニットの初期化
	※初期化と同時にWDTは動き出す。

	[引数]	なし
	[戻値]	なし

	指定のパラメータでWDTユニットを初期化し、必要ならば動作開始させる。

	本関数はcore.h内で定義されている以下のシンボルに基づき初期設定する。
	各シンボルの詳細はcore.hを参照のこと。

	・WWDT_MODE			WDT動作モード
	・WWDT_FREQ			WDT用オシレータの周波数
	・WWDT_DIV			WDT用オシレータ出力時の分周値
	・WWDT_TIM_OUT		WDTタイムアウト時間
	・WWDT_TIM_GUARD	WDTクリアガード時間
	・WWDT_TIM_WARN		WDT警告割り込み発生時間

***************************************************************************/
void Wdt_ini(void)
{
	/* WDTOSCCTRL_Valの通りに周波数を設定し、電源・クロック供給開始 */
	LPC_SYSCON->WDTOSCCTRL = Wdt_calcOscCtrl(WWDT_FREQ, WWDT_DIV);
	LPC_SYSCON->PDRUNCFG &= ~SYS_WDTOSC_PD;			/* 電源オン */
	LPC_SYSCON->SYSAHBCLKCTRL |= SYS_AHB_CLK_WWDT;	/* クロック供給 */

	/* WDTカウンタ・ウィンドウカウンタを指定値で初期化 */
	LPC_WWDT->TC = Wdt_getMs(WWDT_TIM_OUT, WWDT_CNT_MAX);
	LPC_WWDT->WINDOW = Wdt_getMs(WWDT_TIM_OUT-WWDT_TIM_GUARD, WWDT_WINDOW_MAX);
	NVIC_EnableIRQ(WDT_IRQn);

	LPC_WWDT->MOD = WWDT_MODE;
	Wdt_clr();	/* クリア(WDTカウンタ(TV)を設定)することによりWDTが動作開始する */

	/* ※WDTカウンタ(TV)設定後にWARNINTを設定しないと割り込み発生の危険あり */
	LPC_WWDT->WARNINT = Wdt_getMs(WWDT_TIM_WARN, WWDT_WARN_MAX);
}

/***************************************************************************
	Wdt_calcOscCtrl
	WDTOSCCTRLレジスタへの設定値計算

	[引数]	freq	周波数選択値(WDTOSC_FREQ_*のどれか)
			div		分周値(2～64、必ず偶数)
	[戻値]	WDTOSCCTRLレジスタへの設定値

	SYSCON(System configure)中のWDTOSCCTRLレジスタへの設定値を求め、内部エリ
	アを更新する。
	内部エリアは、他のWdt関連の関数が使用するので、本関数を一番最初に呼び出し
	ておく必要がある。
	引数のエラーチェックは行ってない。これらが定義範囲内であることは呼び出し
	側の債務である。
***************************************************************************/
static uint32_t Wdt_calcOscCtrl(uint32_t freq, uint32_t div)
{
	/* UM10601 - 4.6.6 Watchdog oscillator control register参照 */
	enum {
		FREQ_POS		= 5,	/* FREQSELのビット位置 */
		DIVSEL_OFFSET	= 2,	/* DIVSEL=0に相当する分周値 */
		DIVSEL_RATE		= 2		/* DIVSELから分周値を求める時の係数 */
	};

	/* LPC810内蔵のWDTオシレータベース周波数(Hz) */
	/* UM10601 - 4.6.6 Watchdog oscillator control register参照 */
	const uint32_t freqsel[] = {
		[WDTOSC_FREQ_DIS]		= 0,
		[WDTOSC_FREQ_600KHZ]	= 600000,	/* 600kHz */
		[WDTOSC_FREQ_1_05MHZ]	= 1050000,	/* 1.05MHz */
		[WDTOSC_FREQ_1_40MHZ]	= 1400000,	/* 1.40MHz */
		[WDTOSC_FREQ_1_75MHZ]	= 1750000,	/* 1.75MHz */
		[WDTOSC_FREQ_2_10MHZ]	= 2100000,	/* 2.10MHz */
		[WDTOSC_FREQ_2_40MHZ]	= 2400000,	/* 2.40MHz */
		[WDTOSC_FREQ_2_70MHZ]	= 2700000,	/* 2.70MHz */
		[WDTOSC_FREQ_3_00MHZ]	= 3000000,	/* 3.00MHz */
		[WDTOSC_FREQ_3_25MHZ]	= 3250000,	/* 3.25MHz */
		[WDTOSC_FREQ_3_50MHZ]	= 3500000,	/* 3.50MHz */
		[WDTOSC_FREQ_3_75MHZ]	= 3750000,	/* 3.75MHz */
		[WDTOSC_FREQ_4_00MHZ]	= 4000000,	/* 4.00MHz */
		[WDTOSC_FREQ_4_20MHZ]	= 4200000,	/* 4.20MHz */
		[WDTOSC_FREQ_4_40MHZ]	= 4400000,	/* 4.40MHz */
		[WDTOSC_FREQ_4_60MHZ]	= 4600000	/* 4.60MHz */
	};

	Wdt_freq = freqsel[freq];
	Wdt_div = div;
	return (freq << FREQ_POS) | ((div - DIVSEL_OFFSET) / DIVSEL_RATE);
}

/***************************************************************************
	Wdt_getMs
	指定時間に対応するWDTカウンタ値の取得
	※あらかじめWdt_calcOscCtrlを呼び出しておくこと

	[引数]	ms	指定時間(ms)
			max	カウンタ上限値
	[戻値]	WDTカウンタ値

	WDTオシレータの周波数(Hz)をベースに、指定時間(ms)をWDTカウンタ値に
	変換する。
	カウンタ上限値を超えたら上限値に丸める。
	WDTの秒数は概して大きくなりがちなので、桁あふれしないようuint64_tを使用
	している。
***************************************************************************/
static uint32_t Wdt_getMs(uint32_t ms, uint32_t max)
{
	/* UM10601 - 12.6.4 Watchdog Timer Constant register参照 */
	enum {
		PRE_DIV		= 4,	/* プリスケーラの分周値(固定値) */
		SEC_UINT	= 1000	/* カウンタ係数；1秒単位の場合は1、1ms単位の場合は1000... */
	};

	uint64_t cnt = ((uint64_t)Wdt_freq * ms) / (Wdt_div * PRE_DIV * SEC_UINT);
	return (cnt > max)? max: (uint32_t)cnt;
}

/***************************************************************************
	Wdt_getOscClk
	WDT用オシレータの周波数取得
	※あらかじめWdt_calcOscCtrlを呼び出しておくこと

	[引数]	なし
	[戻値]	WDT用オシレータの周波数(Hz)を返す。

	SYSCONユニットのWDTOSCCTRLレジスタの設定値より、最終的に選択される
	オシレータの周波数を求める。
***************************************************************************/
uint32_t Wdt_getOscClk(void)
{
	return Wdt_freq / Wdt_div;
}

/***************************************************************************
	Wdt_clr
	WDTのクリア

	[引数]	なし
	[戻値]	なし

	WDTをクリアするので定期的に呼び出すこと。
***************************************************************************/
void Wdt_clr(void)
{
	/* UM10601 - 12.6.3 Watchdog Feed register参照 */
	LPC_WWDT->FEED = 0xAA;
	LPC_WWDT->FEED = 0x55;
}
