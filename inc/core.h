/***************************************************************************
	core.h
	ファームウェア基本定義

	マイコン: LPC810(NXP Semiconductors)

	本ヘッダを取り込めば、LPC810マイコンプログラムするための基本的な定義を
	使えるようにしている。

	また、本ヘッダ内のシンボルで、プログラムの動作選択を行う。

	変更履歴
	2014.06.07: mits: 新規作成
***************************************************************************/
#ifndef	CORE_H
#define	CORE_H

/***************************************************************************
	システム共通インクルードファイル
***************************************************************************/
#include	<stdint.h>		/* C99型定義 */
#include	<stdbool.h>		/* ブール型定義(C99) */
#include	"LPC8xx.h"		/* LPC800関連の定義 */
#include	"lpc8xx_ctrl.h"	/* LPC800関連の定義追加版 */

/***************************************************************************
	クロック周波数の定義(Sys_lib.c内で使用)

	LPC8xxマイコンを使用する場合、IRC_HZはこのままで良い。
	ソースクロックをCLKIN端子からのクロック入力にする場合は、CLKIN_HZを入力
	するクロック周波数へと変更すること。
***************************************************************************/
enum {
	IRC_HZ		= 12000000,	/* Hz; 内蔵オシレータ周波数 */
	CLKIN_HZ	= 12000000	/* Hz; CLKIN端子からの入力クロック */
};

/***************************************************************************
	システムクロックの選択コード(Sys_lib.c内で使用)

	以下のシンボルに基づきクロックを選択する。

	・MAIN_CLK_SEL: メインクロックの選択(以下のどれか)
		SYS_MAIN_CLK_IRC	内蔵オシレータ(周波数はIRC_HZ)
		SYS_MAIN_CLK_WDTOSC	WDT用オシレータ
		SYS_MAIN_CLK_PLLIN	PLLへの入力クロック
		SYS_MAIN_CLK_PLLOUT	PLLからの出力クロック

	・SYS_PLL_CLK: PLL入力クロックの選択(以下のどれか)
		SYS_PLL_CLK_IRC		内蔵オシレータ(周波数はIRC_HZ)
		SYS_PLL_CLK_CLKIN	CLKIN端子(周波数はCLKIN_HZ)

	・SYS_PLL_RATE: PLL逓倍数(1～32)

	・SYS_CLK_DIV: システムクロックの分周値(1～255)

	これらユニットの構成は、以下のページで詳しく説明しているので参考にして
	頂きたい。
		http://mits-whisper.info/post/83092289364/lpc810-10-1
	なお、このページでも触れているが、PLL設定のPost divider(SYSPLLCTRL[PSEL])
	は設定しても動作しないため、これらシンボルには含めてない。

	例1) 内蔵オシレータで動かす場合
		MAIN_CLK_SEL = SYS_MAIN_CLK_IRC

	例2) WDT用オシレータで動かす場合
		MAIN_CLK_SEL = SYS_MAIN_CLK_WDTOSC
		※WDT用オシレータのクロック周波数は別シンボル(WWDT_FREQ, WWDT_DIV)
		　で指定する(本ファイル内にあり)。

	例3) CLKIN端子からの入力クロックで動かす場合
		MAIN_CLK_SEL = SYS_MAIN_CLK_PLLIN
		SYS_PLL_CLK  = SYS_PLL_CLK_CLKIN

	例4) 内蔵オシレータを2倍に逓倍して動かす場合
		MAIN_CLK_SEL = SYS_MAIN_CLK_PLLOUT
		SYS_PLL_CLK  = SYS_PLL_CLK_IRC
		SYS_PLL_RATE = 2
		※LPC8xxシリーズ仕様上の最高速は30MHzである。

	例5) CLKIN端子からの入力クロックを2倍に逓倍して動かす場合
		MAIN_CLK_SEL = SYS_MAIN_CLK_PLLOUT
		SYS_PLL_CLK  = SYS_PLL_CLK_CLKIN
		SYS_PLL_RATE = 2
		※LPC8xxシリーズ仕様上の最高速は30MHzである。

	※すべてのケースでSYS_CLK_DIVは独立して機能する。
***************************************************************************/
enum {
	MAIN_CLK_SEL	= SYS_MAIN_CLK_IRC,		/* メインクロック選択 */
	SYS_PLL_CLK		= SYS_PLL_CLK_IRC,		/* PLL入力クロックの選択 */
	SYS_PLL_RATE	= 1,					/* PLL逓倍数 */
	SYS_CLK_DIV		= 1						/* システムクロックの分周値 */
};

/***************************************************************************
	内蔵オシレータ電源断　選択スイッチ(Sys_lib.c内で使用)

	内蔵オシレータを使用しないクロック選択の場合、オシレータの電源を切るか
	どうかを選択する。
	ただし、切ってしまうと、例えばCLKOUTが出なくなったりするので、自分の使
	いたい機能がちゃんと動作するかどうかを確認してから切ること。
***************************************************************************/
enum {
	IRC_PDWON	= 1		/* 0:切らない、1:切る */
};

/***************************************************************************
	WDT動作モードの指定(Wdt_lib.c内で使用)

	WWDTユニットのMODレジスタへの設定値。
	以下のシンボルの組み合わせ(論理和)で指定する。

	・WWDT_WDEN		WDTを有効にする。
	・WWDT_WDRESET	WDT満了時にシステムリセットさせる。
					指定しないとリセットせずに割り込みが発生する。
					割り込みハンドラは警告割り込みハンドラが使われる。

	例1) WDTを動作させない。※デバッガ接続時
		WWDT_MODE = 0

	例2) WDT満了でシステムリセット
		WWDT_MODE = WWDT_WDEN | WWDT_WDRESET

	例3) WDT満了で割り込みのみ発生
		WWDT_MODE = WWDT_WDEN
***************************************************************************/
enum {
//	WWDT_MODE	= 0
	WWDT_MODE	= WWDT_WDEN | WWDT_WDRESET
//	WWDT_MODE	= WWDT_WDEN
};

/***************************************************************************
	WDT用オシレータの選択値(Wdt_lib.c内で使用)

	以下のシンボルに基づきクロック選択する。

	・WWDT_FREQ: オシレータの周波数(以下のどれか)
		WDTOSC_FREQ_600KHZ .... 600kHz
		WDTOSC_FREQ_1_05MHZ ... 1.05MHz
		WDTOSC_FREQ_1_40MHZ ... 1.40MHz
		WDTOSC_FREQ_1_75MHZ ... 1.75MHz
		WDTOSC_FREQ_2_10MHZ ... 2.10MHz
		WDTOSC_FREQ_2_40MHZ ... 2.40MHz
		WDTOSC_FREQ_2_70MHZ ... 2.70MHz
		WDTOSC_FREQ_3_00MHZ ... 3.00MHz
		WDTOSC_FREQ_3_25MHZ ... 3.25MHz
		WDTOSC_FREQ_3_50MHZ ... 3.50MHz
		WDTOSC_FREQ_3_75MHZ ... 3.75MHz
		WDTOSC_FREQ_4_00MHZ ... 4.00MHz
		WDTOSC_FREQ_4_20MHZ ... 4.20MHz
		WDTOSC_FREQ_4_40MHZ ... 4.40MHz
		WDTOSC_FREQ_4_60MHZ ... 4.60MHz

	・WWDT_DIV: オシレータ出力時の分周値
		2～64の範囲の「偶数」のみ

	例1) 一番遅い周波数(9.375kHz)
		WWDT_FREQ = WDTOSC_FREQ_600KHZ
		WWDT_DIV  = 64

	例2) 一番早い周波数(2.3MHz)
		WWDT_FREQ = WDTOSC_FREQ_4_60MHZ
		WWDT_DIV  = 2
***************************************************************************/
enum {
	WWDT_FREQ	= WDTOSC_FREQ_600KHZ,	/* 周波数：WDTOSC_FREQ_*を指定 */
	WWDT_DIV	= 64					/* 分周値：2～64(偶数のみ) */
};

/***************************************************************************
	WWDT時間(ms)の指定(Wdt_lib.c内で使用)

	以下のシンボルに基づきWWDT時間を初期化する。

	・WWDT_TIM_OUT
		WDTタイムアウト時間。
		この時間分WDTのクリアが成されないとWDT満了となる。
		ms単位で指定する。
		指定可能な時間はWDT用オシレータのクロック周波数に依存する。
		200(=200ms)～29000(=29秒)の範囲内ならば、すべてのクロック周波数で
		指定可能である。

	・WWDT_TIM_GUARD
		WDTクリアのガード時間。
		クリア後、この時間は再クリアできない。
		クリアすると、WDT満了と同じ扱いになる。
		ms単位で指定する。指定範囲は0～WWDT_TIM_OUTである。
		0にすると常時クリア可能となる。

	・WWDT_TIM_WARN
		WDT警告割り込み発生時間。
		WDTが残りこの時間になると警告割り込みが発生する。
		0にするとWDT満了時に警告割り込み発生となる。
		指定可能な時間はWDT用オシレータのクロック周波数に依存する。
		一番遅いクロック設定(9.375kHz)だった場合は、最大400(=400ms)までは
		指定可能である。
		一番早いクロック設定(2.3MHz)だった場合は、1(=1ms)しか指定できない。

	※WWDT_TIM_OUTとWWDT_TIM_WARNの指定範囲に関しては、以下のページで詳しく
	　説明している。
		http://mits-whisper.info/post/85408704581/lpc810-14
***************************************************************************/
enum {
	WWDT_TIM_OUT	= 2000,		/* ms; WDTタイムアウト時間 */
	WWDT_TIM_GUARD	= 0,		/* ms; WDTクリアガード時間 */
	WWDT_TIM_WARN	= 200		/* ms; 警告発生時間 */
};

#endif	/* CORE_H */
