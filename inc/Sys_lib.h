/***************************************************************************
	Sys_lib.h
	私家版システムライブラリ

	マイコン: LPC810(NXP Semiconductors)

	変更履歴
	2014.06.07: mits: 新規作成
***************************************************************************/
#ifndef	SYS_LIB_H
#define	SYS_LIB_H

/***************************************************************************
	グローバル関数
***************************************************************************/
void		Sys_iniLpc810(void);
uint32_t	Sys_getMainClk(void);
uint32_t	Sys_getSysClk(void);

/***************************************************************************
	以下は、コアライブラリとの整合性をとるためのextern宣言
***************************************************************************/
extern uint32_t SystemCoreClock;
void SystemCoreClockUpdate(void);

#endif	/* SYS_LIB_H */