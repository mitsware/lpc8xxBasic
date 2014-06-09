/***************************************************************************
	Sys_lib.c
	私家版システムライブラリ

	使用方法: #include "Sys_lib.h"

	マイコン: LPC810(NXP Semiconductors)

	コアライブラリのSystemInitで感じた問題に対処した私家版のシステム設定
	ライブラリ。
	現版では、外付けの水晶発振子をサポートしてないため、LPC810専用である。

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

	変更履歴
	2014.06.07: mits: 新規作成
***************************************************************************/
#include	"core.h"
#include	"Sys_lib.h"
#include	"Wdt_lib.h"	/* for Wdt_* */

/***************************************************************************
	ローカル変数
***************************************************************************/
static uint32_t	Sys_mainClk;	/* メインクロック(Sys_iniLpc810で初期化) */

/***************************************************************************
	コアライブラリオリジナルスタブ
***************************************************************************/
uint32_t	SystemCoreClock;
void SystemCoreClockUpdate(void)
{
	/* 特にやる仕事なし */
}

/***************************************************************************
	Sys_iniLpc810
	システムクロック設定

	[引数]	なし
	[戻値]	なし

	LPC810用のシステムクロック選択を行う。
	コアライブラリのSystemInitを置き換えるものとして作った。

	core.h内で定義されている以下のシンボルの内容に基づきクロックを選択する。
	各シンボルの詳細はcore.hを参照のこと。

	・MAIN_CLK_SEL	メインクロックの選択
	・SYS_PLL_CLK	PLL入力クロックの選択
	・SYS_PLL_RATE	PLL逓倍数
	・SYS_CLK_DIV	システムクロックの分周値

	また、内蔵オシレータを使用しないクロック選択の場合は、内蔵オシレータの電
	源を落とすことができるようにした。
	core.h内で定義されているIRC_PDWONの定義状態に基づく。
	core.h内の説明を参照のこと。

	既存のSystemInitを置き換えるにあたって、以下の改善を施している。

	・PLLの安定などでビジーウェイトしているため、ウォッチドッグタイマで保護
	　するようにした。

	・本関数内でシステムクロック値を示す外部変数SystemCoreClockを設定するよ
	　うにした。
	　もはやSystemCoreClockUpdate()を呼び出す必要は無い。
	　なお、システムクロックはSys_getSysClk()で取得できる。

	・メインクロック値も残すようにした。
	　Sys_getMainClk()で取得できる。
	　SYS_CLK_DIVに2以上の分周値を設定すると、システムクロックと異なる値に
	　なる。

	なお、PLL設定のPost divider(SYSPLLCTRL[PSEL])は設定しても動作しないため、
	本関数内では処理を行ってない。
	この件に関しては以下のページで詳しく述べている。
		http://mits-whisper.info/post/83092289364/lpc810-10-1
***************************************************************************/
void Sys_iniLpc810(void)
{
	enum {
		PLL_OFFSET	= 1,	/* SYSPLLCTRLでMSEL=0の時の逓倍数 */
							/* ※UM10601 - 4.6.3 System PLL control register */
		SYSCON_WAIT	= 200	/* レジスタ設定が安定するまでのWaitカウント値 */
							/* ※マニュアルに記載なし(SystemInitを参考) */
	};
	volatile uint32_t	i;
	_Bool				pllirc = true;		/* PLLの入力は内蔵オシレータ */
	uint32_t			pllsrc = IRC_HZ;	/* PLL入力クロック数 */
	_Bool				irc = true;			/* 内蔵オシレータを使う */

	/* 最初にウォッチドッグタイマを初期化し開始する */
	Wdt_ini();

	/* 基本ユニット(SWM, IOCON)にクロック供給 */
	LPC_SYSCON->SYSAHBCLKCTRL |= SYS_AHB_CLK_SWM | SYS_AHB_CLK_IOCON;

	/* CLKINが選択されていた場合 */
	if ((SYS_PLL_CLK & SYS_PLL_CLK_SEL) == SYS_PLL_CLK_CLKIN) {
		LPC_IOCON->PIO0_1 &= ~IOCON_MODE;		/* プルアップ/ダウン抵抗を外す */
		LPC_SWM->PINENABLE0 &= ~SWM_CLKIN_DIS;	/* CLKIN端子を有効化 */
		for (i = 0; i < SYSCON_WAIT; i++) {
			__NOP();
		}
		pllsrc = CLKIN_HZ;
		pllirc = false;		/* PLLの入力は内蔵オシレータでない */
	}

	LPC_SYSCON->SYSPLLCLKSEL = SYS_PLL_CLK;			/* PLL入力クロックの選択 */
	LPC_SYSCON->SYSPLLCLKUEN = SYS_PLL_CLK_UPDATE;	/* PLL動作開始 */
	while ((LPC_SYSCON->SYSPLLCLKUEN & SYS_PLL_CLK_UPDATE) != SYS_PLL_CLK_UPDATE) {
		;	/* 安定するまで待機 */
	}

	switch (MAIN_CLK_SEL & SYS_MAIN_CLK_SEL)
	{
	case SYS_MAIN_CLK_IRC:		/* メインクロックに内蔵オシレータを選択 */
		Sys_mainClk = IRC_HZ;
		break;
	case SYS_MAIN_CLK_PLLIN:	/* メインクロックにPLL入力クロックを選択 */
		Sys_mainClk = pllsrc;
		if (!pllirc) {
			irc = false;
		}
		break;
	case SYS_MAIN_CLK_PLLOUT:	/* メインクロックにPLL出力クロックを選択 */
		LPC_SYSCON->SYSPLLCTRL = SYS_PLL_RATE - PLL_OFFSET;	/* 逓倍数の設定 */
		LPC_SYSCON->PDRUNCFG &= ~SYS_SYSPLL_PD;				/* PLLに電源供給 */
		while ((LPC_SYSCON->SYSPLLSTAT & SYS_PLL_STAT) != SYS_PLL_LOCKED) {
			;	/* 安定するまで待機 */
		}
		Sys_mainClk = pllsrc * SYS_PLL_RATE;
		if (!pllirc) {
			irc = false;
		}
		break;
	case SYS_MAIN_CLK_WDTOSC:	/* メインクロックにWDT用オシレータを選択 */
		Sys_mainClk = Wdt_getOscClk();
		irc = false;
		break;
	default:
		break;
	}

	LPC_SYSCON->MAINCLKSEL = MAIN_CLK_SEL;			/* メインクロックの選択 */
	LPC_SYSCON->MAINCLKUEN = SYS_MAIN_CLK_UPDATE;	/* メインクロック更新開始 */
	while ((LPC_SYSCON->MAINCLKUEN & SYS_MAIN_CLK_UPDATE) != SYS_MAIN_CLK_UPDATE) {
		;	/* 安定するまで待機 */
	}

	if (IRC_PDWON && !irc) {	/* 内蔵オシレータを使わない場合は電源オフ */
		LPC_SYSCON->PDRUNCFG |= SYS_IRCOUT_PD | SYS_IRC_PD;
	}

	/* システムクロック分周値の設定 */
	LPC_SYSCON->SYSAHBCLKDIV = SYS_CLK_DIV;
	SystemCoreClock = Sys_mainClk / SYS_CLK_DIV;
}

/***************************************************************************
	Sys_getMainClk
	メインクロック値の取得
	※あらかじめSys_iniLpc810を呼び出しておくこと。

	[引数]	なし
	[戻値]	メインクロック周波数(Hz)を返す。
***************************************************************************/
uint32_t Sys_getMainClk(void)
{
	return Sys_mainClk;
}

/***************************************************************************
	Sys_getSysClk
	システムクロック値の取得
	※あらかじめSys_iniLpc810を呼び出しておくこと。

	[引数]	なし
	[戻値]	システムクロック周波数(Hz)を返す。
***************************************************************************/
uint32_t Sys_getSysClk(void)
{
	return SystemCoreClock;
}
