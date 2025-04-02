// SPDX-FileCopyrightText: Copyright (C) 2022,2023,2024,2025  Hajime Yamaguchi
// SPDX-License-Identifier: GPL-3.0-or-later

#include	<stdio.h>
#include	<stdlib.h>
#include	<stdbool.h>
#include	<unistd.h>
#include	<libgen.h>
#include	<string.h>
#include	<gmp.h>

#include	"shiika_man.h"

char	input_buffer[ BUFFER_SIZE ];	// 入力バッファ
char	str_buffer[ BUFFER_SIZE ];		// 表示用文字バッファ
int		ka[ NUM_OF_CHAR ];				// 歌の配列

int shiika_start_menu( void ){
	int		input_num;

	do{
		printf( "■ 詩歌マネージャ  Version %s\n", VERSION );
		printf( "\t%d : 通し番号検索（文字列を入力して番号を検索）\n", SHIIKA_MODE_S2N  );
		printf( "\t%d : 文字列　検索（番号を入力して文字列を検索）\n", SHIIKA_MODE_N2S  );
		printf( "\t%d : 使い方説明\n"                                , SHIIKA_MODE_HELP );
		printf( "\t%d : 終了\n"                                      , SHIIKA_MODE_EXIT );
		fgets( input_buffer, BUFFER_SIZE, stdin );
		input_num = atoi( input_buffer );
		if( debug_mode ){ printf( "入力番号：%d\n", input_num ); }								///DBG
	} while( ( input_num < SHIIKA_MODE_EXIT ) || ( SHIIKA_MODE_MAX <= input_num ) );
	return input_num;
}

int shiika_end_menu( void ){
		printf( "\n" );
		printf( "\t<Enter> : 継続\n" );
		printf( "\t0       : 終了（メニュー）\n" );
		fgets( input_buffer, BUFFER_SIZE, stdin );
		return atoi( input_buffer );
}

int grammar_check( int ka[] ){
	// 文法チェック
	//	文法チェックを行う
	// リターン値
	//		GRM_OK              : 正常
	//		GRM_HATSUON_TOP     : 撥音「ん」で始まっている
	//		GRM_SOKUON_TOP      : 促音「っ」で始まっている
	//		GRM_SOCUON_CONTINUE : 促音「っ」が続いている

	int		i;
	bool	pre_sokuon;

	if( debug_mode ){ gmp_printf( "文法チェックd\n" ); }										///DBG
	if( strcmp( moji[ka[0]], "ん" ) == 0 ){
		printf( "文法エラー：撥音「ん」で始まっています。\n" );
		return GRM_HATSUON_TOP;
	}
	if( strcmp( moji[ka[0]], "っ" ) == 0 ){
		printf( "文法エラー：促音「っ」で始まっています。\n" );
		return GRM_SOKUON_TOP;
	}
	if( debug_mode ){ gmp_printf( "%2d %s\n", 0, moji[ka[0]] ); }								///DBG

	pre_sokuon = false;
	for( i=1; i<NUM_OF_CHAR; i++ ){
		if( debug_mode ){ gmp_printf( "%2d %s\n", i, moji[ka[i]] ); }							///DBG
		if( strcmp( moji[ka[i]], "っ" ) == 0 ){
			if( pre_sokuon ){
				printf( "文法エラー：促音「っ」が続いています。\n" );
				return GRM_SOCUON_CONTINUE;
			} else {
				pre_sokuon = true;
			}
		} else {
			pre_sokuon = false;
		}
	}
	if( debug_mode ){ gmp_printf( "\n" ); }														///DBG

	return GRM_OK;
}

int	convert_num_unit( char *input_str, char *output_str ){
	// 数値変換
	//	数値を４桁毎に区切って、数値の単位を付加する

	char		*str = input_str;
	char		*wp  = output_str;
	u_int32_t	len, len_sub;
	int32_t		keta;

	len = strlen( str );

	// Error チェック
	if( len > 68 ){
		printf( "桁が長すぎます。\n" );
		return -1;
	}

	keta = len / 4;
	len_sub = len % 4;
	if( len_sub == 0 ){		// 桁数が４の倍数の時のみ
		keta = keta -1;
		len_sub = 4;
	}
	if( debug_mode ){ printf( "len : %d, len_sub : %d\n", len, len_sub ); }						///DBG

	do{
		if( debug_mode ){ printf( "keta : %d, num_unit[keta] : '%s', str : '%s'\n", 			///DBG
									keta, num_unit[keta], str ); }								///DBG

		strncpy( wp, str, len_sub );
		wp += len_sub;
		*wp = '\0';
		strcat( wp, num_unit[keta] );
		wp += strlen( wp );
		str += len_sub;

		len_sub = 4;
		keta--;

		if( debug_mode ){ printf( "keta : %d,  output_str : '%s'\n", keta, output_str ); }		///DBG
	}while( keta >= 0 );

	return 0;
}

int shiika_str_to_num_f( void ){
	// 文字列入力
	// エラーチェック
	// 文字列→番号変換
	// 		文字に分解、歌の配列に格納
	// 		番号の計算
	// 表示
	// 継続・終了メニュー

	int		input_num;
	char	*p;
	int		i, j;
	int		len;
	bool	hit;
	char	*input_mes;

	printf( "【通し番号検索】\n");
	do{
		// 入力
		printf( "歌・句を入力してください\n" );
		fgets( input_buffer, BUFFER_SIZE, stdin );
		if( debug_mode ){ printf( "入力文字列：「%s」\n", input_buffer ); }						///DBG

		// 文字切り出し
		p = input_buffer;
		for( i=0; i<NUM_OF_CHAR; i++ ){
			if( *p == '\n' ){
				// 字足らず
				for( j=i; j<NUM_OF_CHAR; j++ ){
					ka[j] = 0;
				}
				break;
			}
			hit = false;
			for( j=n_size-1; j>=0; j-- ){
				len = strlen( moji[j] );
//				if( debug_mode ){ printf( "j : %3d, moji[j] :「%s」,", j, moji[j] ); }		///DBG
//				if( debug_mode ){ printf( "len : %d, ", len ); }							///DBG
//				if( debug_mode ){ printf( "p :「%s」\n", p ); }								///DBG
				if( strncmp( moji[j], p, len ) == 0 ){
					ka[i] = j;
					p += len;
					if( debug_mode ){ printf( "残り : %s, *p : 0x%02X\n", p, *p ); }				///DBG
					hit = true;
					break;
				}
			}
			if( ! hit ){
				printf( "Error : 不適格な文字が含まれています\n\n" );
				return -1;
			}
		}
		if( i < NUM_OF_SENRYU ){
			input_mes = "[俳句・川柳] 文字が短すぎます（字足らずで処理します）";
		} else if( i == NUM_OF_SENRYU ){
			input_mes = "[俳句・川柳]";
		} else if( i <NUM_OF_DODOITSU ){
			input_mes = "[都々逸] 文字が短すぎます（字足らずで処理します）";
		} else if( i == NUM_OF_DODOITSU ){
			input_mes = "[都々逸]";
		} else if( i <NUM_OF_CHAR ){
			input_mes = "[短歌] 文字が短すぎます（字足らずで処理します）";
		} else {
			input_mes = "[短歌]";
		}
		if( *p != '\n' ){
			printf( "Warning : 文字が長すぎます（切り捨てて処理します）\n" );
		}

		// 文法チェック
		grammar_check( ka );

		if( debug_mode ){													///DBG
			for( i=0; i<NUM_OF_CHAR; i++ ){									///DBG
				printf( "%d\t%s\n", ka[i], moji[ka[i]] );					///DBG
			}																///DBG
		}																	///DBG

		// 通し番号計算
		mpz_t	mp_number;
		mpz_t	mp_n_size;
		mpz_t	mp_ka;

		mpz_init( mp_number );					// mp_number = 0;
		mpz_init( mp_ka );
		mpz_init( mp_n_size );
		mpz_set_si( mp_n_size, n_size );
		if( debug_mode ){ gmp_printf( "mp_n_size : %Zd\n",  mp_n_size ); }											///DBG

		for( i=NUM_OF_CHAR-1; i>=0; i-- ){
//		for( i=0; i<NUM_OF_CHAR; i++ ){
			if( debug_mode ){ gmp_printf( "pre  mp_number : %Zd,  mp_n_size : %Zd\n", mp_number, mp_n_size ); }		///DBG
			mpz_set_si( mp_ka, ka[i] );
			mpz_mul( mp_number, mp_number, mp_n_size );
			if( debug_mode ){ gmp_printf( "mid  mp_number : %Zd,  mp_ka : %Zd\n", mp_number, mp_ka ); }				///DBG
			mpz_add( mp_number, mp_number, mp_ka );
			if( debug_mode ){ gmp_printf( "post mp_number : %Zd\n", mp_number ); }									///DBG
		}

		// 表示
		printf( "\n" );
		printf( "入力文字列：" );
		for( i=0; i<NUM_OF_CHAR; i++ ){
			printf( "%s", moji[ ka[ i ] ] );
		}
		printf( "\n" );
		printf( "分類　　　：%s\n", input_mes );
		mpz_get_str( input_buffer, 10, mp_number );
		convert_num_unit( input_buffer, str_buffer );
		printf( "通し番号　：" );
		gmp_printf( "%Zd\n", mp_number );
		printf( "　　　　　：%s\n", str_buffer );

		// 解放
		mpz_clear( mp_number );
		mpz_clear( mp_ka );
		mpz_clear( mp_n_size );

		// 継続・終了メニュー
		input_num = shiika_end_menu();

	} while( ( input_num == 0 ) && ( *input_buffer != '0' ) );		// 空打ちのとき true、継続。　本当に 0 入力時 false。
																	// ★ この部分は後で見直す。
}


int shiika_num_to_str_f( void ){
	// 番号入力
	// エラーチェック(?)
	// 剰余の計算による、各文字の決定
	// エラーチェック（こっちでやった方がいい？）
	// 表示
	// 継続・終了メニュー

	int			input_num;
	u_int32_t	i, j;
	char	*output_mes;

	printf( "【文字列検索】\n");
	do{
		// 入力
		printf( "通し番号を入力してください\n" );
		fgets( input_buffer, BUFFER_SIZE, stdin );
		if( debug_mode ){ printf( "通し番号：「%s」\n", input_buffer ); }						///DBG

		mpz_t	mp_number;
		mpz_t	mp_number_bak;
		mpz_t	mp_number_max;
		mpz_t	mp_n_size;
		mpz_t	mp_ka;

		mpz_init( mp_number );
		mpz_init( mp_number_bak );
		mpz_init( mp_number_max );
		mpz_init( mp_n_size );
		mpz_init( mp_ka );
		mpz_set_si( mp_n_size, n_size );
		mpz_ui_pow_ui( mp_number_max, n_size, NUM_OF_CHAR);
		if( mpz_set_str( mp_number, input_buffer, 10) <0 ){
			printf( "Error : 数値以外が指定されています\n" );
			return -1;
		}
		mpz_set( mp_number_bak, mp_number);

		if( debug_mode ){ gmp_printf( "mp_number_max : %Zd, mp_n_size : %Zd\n",  mp_number_max, mp_n_size ); }		///DBG
		if( debug_mode ){ gmp_printf( "input mp_number : %Zd\n",  mp_number ); }									///DBG

		// エラーチェック
		if( mpz_cmp( mp_number_max, mp_number) <= 0 ){
			printf( "Error : 番号が大きすぎます\n" );
			return -1;
		}

		// 各文字の決定
		for( i=0; i<NUM_OF_CHAR; i++ ){
			mpz_mod( mp_ka, mp_number, mp_n_size );
			mpz_tdiv_q( mp_number, mp_number, mp_n_size );
			ka[i] = mpz_get_ui( mp_ka );
			if( debug_mode ){ gmp_printf( "post mp_number : %3Zd, mp_ka : %Zd, moji[ka[mp_ka]] : %s\n",				///DBG
																	mp_ka, mp_number, moji[ka[i]] ); }				///DBG
			if( ( mpz_get_ui( mp_number ) == 0 ) && ( ka[i] == 0 ) ){
				for( j=i+1; j<NUM_OF_CHAR; j++ ){
					ka[j] = 0;
				}
				break;
			}
		}
		if( i < NUM_OF_SENRYU ){
			output_mes = "[俳句・川柳] 字足らずです";
		} else if( i == NUM_OF_SENRYU ){
			output_mes = "[俳句・川柳]";
		} else if( i <NUM_OF_DODOITSU ){
			output_mes = "[都々逸] 文字が短すぎます（字足らずで処理します）";
		} else if( i == NUM_OF_DODOITSU ){
			output_mes = "[都々逸]";
		} else if( i <NUM_OF_CHAR ){
			output_mes = "[短歌] 字足らずです";
		} else {
			output_mes = "[短歌]";
		}

		// 表示
		mpz_get_str( input_buffer, 10, mp_number_bak );
		convert_num_unit( input_buffer, str_buffer );
		printf( "\n" );
		printf( "通し番号　：" );
		gmp_printf( "%Zd\n", mp_number_bak );
		printf( "　　　　　：%s\n", str_buffer );
		printf( "分類　　　：%s\n", output_mes );
		printf( "文字列　　：" );
		for( i=0; i<NUM_OF_CHAR; i++ ){
			printf( "%s", moji[ ka[ i ] ] );
		}
		printf( "\n" );

		// 文法チェック
		grammar_check( ka );

		// 解放
		mpz_clear( mp_number );
		mpz_clear( mp_ka );
		mpz_clear( mp_n_size );

		// 継続・終了メニュー
		input_num = shiika_end_menu();

	} while( ( input_num == 0 ) && ( *input_buffer != '0' ) );		// 空打ちのとき true、継続。　本当に 0 入力時 false。
}


void shiika_help_f( void ){
	printf(
		"詩歌マネージャ\n"
		"￣￣￣￣￣￣￣\n"
		"\n"
		"■ コンセプト\n"
		"　「和歌」、「都々逸」、そして「俳句・川柳」のような文学は、「詩歌」と総称される。\n"
		"\n"
		"　俳句・川柳は 17 文字、都々逸は 26 文字、和歌は 31 文字で表現されるアートだ。\n"
		"　文字の組合せで表現される世界は膨大である。\n"
		"　……だが、無限ではない。\n"
		"\n"
		"\n"
		"　「詩歌」として表現できる言葉の組み合わせは有限であり、番号を通して管理が可能である、\n"
		"と考えることができる。\n"
		"\n"
		"　詩歌マネージャは、世の中に存在しうる「詩歌」に通し番号を割当てる。\n"
		"\n"
		"　任意の詩歌を入力すると、該当する番号を表示する。\n"
		"　逆に、任意の数値を入力すると、該当する詩歌を表示する。\n"
		"\n"
		"　詩歌マネージャ は、この世に存在する「詩歌」を自らの管理下に置く、という途方もない\n"
		"（傲慢な？）コンセプトを実現するプロジェクトである。\n"
		"\n"
		"\n"
		"■ できること\n"
		"□ 通し番号の表示\n"
		"　詩歌を入力し、該当する通し番号を表示する。\n"
		"\n"
		"□ 詩歌の表示\n"
		"　通し番号を入力し、該当する詩歌を表示する。\n"
		"\n"
		"\n"
		"■ 限界\n"
		"　俳句における季語についてはサポートしない。\n"
		"\n"
		"　また、漢字には対応しない。\n"
		"　同音異義語で構成された本来別の俳句・川柳も、本プログラムでは同一のものとして扱う。\n"
		"（手を抜いた部分。　てへ、ぺろ）\n"
		"\n"
		"　字余りについては一切扱わない。\n"
		"\n"
		"　字足らずは一応含める。\n"
		"（その代わり、1 文字～16 文字、18 文字～25 文字、27 文字～30 文字の作品についても\n"
		"　通し番号を割り当て、管理下に置く。）\n"
		"\n"
		"\n"
		"■ 操作方法\n"
		"【メインメニュー】\n"
		">■ 詩歌マネージャ\n"
		">        1 : 通し番号検索（文字列を入力して番号を検索）\n"
		">        2 : 文字列　検索（番号を入力して文字列を検索）\n"
		">        3 : 使い方説明\n"
		">        0 : 終了\n"
		"\n"
		"【コマンド入力】\n"
		"　1<enter> で通し番号検索処理\n"
		"　2<enter> で文字列検索処理\n"
		"　3<enter> で使い方説明（この画面の表示）\n"
		"　0<enter> で終了\n"
		"\n"
		"■ おわり\n"
		" Enger キーを押してください\n"
	);
	fgets( input_buffer, BUFFER_SIZE, stdin );
	return;
}

int main( int argc, char*argv[] ){
	int		mode;
	int		opt;

	// オプションチェック
	while ((opt = getopt(argc, argv, "dt")) != -1) {
		switch (opt) {
		case 'd':	// デバッグモード検出
			debug_mode = true;
			printf( "debug mode on\n" );
			break;
//		case 't':	// テストモード検出
//			test_mode = true;
//			printf( "test mode on\n" );
//			break;
		default:
			printf( "%s [-d]\n", basename( argv[0] ) );
			printf( "\t-d : debug mode\n" );
			return 1;
			break;
		}
	}

	do{
		mode = shiika_start_menu();
		switch( mode ){
		case SHIIKA_MODE_S2N:
			shiika_str_to_num_f();
			break;
		case SHIIKA_MODE_N2S:
			shiika_num_to_str_f();
			break;
		case SHIIKA_MODE_HELP:
			shiika_help_f();
			break;
		case SHIIKA_MODE_EXIT:
			break;
		default:
			break;
		}
	}while( mode != SHIIKA_MODE_EXIT );
	printf( "さようなら\n");
	return 0;
}
