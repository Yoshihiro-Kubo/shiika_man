#ifndef _SHIIKA_MAN_H_
#define	_SHIIKA_MAN_H_

#include	"moji.h"

// Define
#define	NUM_OF_HAIKU		31
#define	NUM_OF_SENRYU		17
#define	NUM_OF_CHAR			NUM_OF_HAIKU

#define	BUFFER_SIZE			2048

#define	SHIIKA_MODE_EXIT	0
#define	SHIIKA_MODE_S2N		1
#define	SHIIKA_MODE_N2S		2
#define	SHIIKA_MODE_HELP	3
#define	SHIIKA_MODE_MAX		4

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


#endif	// _SHIIKA_MAN_H_
