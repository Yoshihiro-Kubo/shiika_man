#ifndef _SENRYU_MAN_H_
#define	_SENRYU_MAN_H_

#include	"moji.h"

// Define
#define	NUM_OF_CHAR	17
#define	BUFFER_SIZE	2048

#define	SENRYU_MODE_EXIT	0
#define	SENRYU_MODE_S2N	1
#define	SENRYU_MODE_N2S	2
#define	SENRYU_MODE_HELP	3
#define	SENRYU_MODE_MAX	4

// 文法チェック
#define	GRM_OK				 0
#define	GRM_HATSUON_TOP		-1
#define	GRM_SOKUON_TOP		-2
#define	GRM_SOCUON_CONTINUE	-3


// Gloval varialbes
// 数値表示用単位
char	*num_unit[]	= {
	"　",
	"万 ",
	"億 ",
	"兆 ",
	"京 ",
	"垓 ",
	"禾予 ",		// 本当は ノ木偏に「予」の一文字。
	"穣 ",
	"溝 ",
	"澗 ",
	"正 ",
	"載 ",
	"極 ",
	"恒河沙 ",
	"阿僧祇 ",
	"那由他 ",
	"不可思議 ",
	"無量大数 ",
};

// mode
bool	debug_mode = false;



#endif	// _SENRYU_MAN_H_
