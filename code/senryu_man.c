/*
	Copyright (C) 2022  Hajime Yamaguchi 
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<stdbool.h>
#include	<unistd.h>
#include	<libgen.h>
#include	<string.h>
#include	<gmp.h>

#include	"senryu_man.h"

char	input_buffer[ BUFFER_SIZE ];	// 入力バッファ
char	str_buffer[ BUFFER_SIZE ];		// 表示用文字バッファ
int		ku[ NUM_OF_CHAR ];				// 句の配列

int senryu_start_menu( void ){
	int		input_num;

	do{
		printf( "■ 川柳・俳句ジェネレータ\n" );
		printf( "\t%d : 通し番号検索（文字列を入力して番号を検索）\n", SENRYU_MODE_S2N  );
		printf( "\t%d : 文字列　検索（番号を入力して文字列を検索）\n", SENRYU_MODE_N2S  );
		printf( "\t%d : 使い方説明\n"                                , SENRYU_MODE_HELP );
		printf( "\t%d : 終了\n"                                      , SENRYU_MODE_EXIT );
		fgets( input_buffer, BUFFER_SIZE, stdin );
		input_num = atoi( input_buffer );
		if( debug_mode ){ printf( "入力番号：%d\n", input_num ); }								///DBG
	} while( ( input_num < SENRYU_MODE_EXIT ) || ( SENRYU_MODE_MAX <= input_num ) );
	return input_num;
}

int senryu_end_menu( void ){
		printf( "\n" );
		printf( "\t<Enter> : 継続\n" );
		printf( "\t0       : 終了（メニュー）\n" );
		fgets( input_buffer, BUFFER_SIZE, stdin );
		return atoi( input_buffer );
}

int grammar_check( int ku[] ){
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
	if( strcmp( moji[ku[0]], "ん" ) == 0 ){
		printf( "文法エラー：撥音「ん」で始まっています。\n" );
		return GRM_HATSUON_TOP;
	}
	if( strcmp( moji[ku[0]], "っ" ) == 0 ){
		printf( "文法エラー：促音「っ」で始まっています。\n" );
		return GRM_SOKUON_TOP;
	}
	if( debug_mode ){ gmp_printf( "%2d %s\n", 0, moji[ku[0]] ); }								///DBG

	pre_sokuon = false;
	for( i=1; i<NUM_OF_CHAR; i++ ){
		if( debug_mode ){ gmp_printf( "%2d %s\n", i, moji[ku[i]] ); }							///DBG
		if( strcmp( moji[ku[i]], "っ" ) == 0 ){
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

int senryu_str_to_num_f( void ){
	// 文字列入力
	// エラーチェック
	// 文字列→番号変換
	// 		文字に分解、句の配列に格納
	// 		番号の計算
	// 表示
	// 継続・終了メニュー

	int		input_num;
	char	*p;
	int		i, j;
	int		len;
	bool	hit;

	printf( "【通し番号検索】\n");
	do{
		// 入力
		printf( "句を入力してください\n" );
		fgets( input_buffer, BUFFER_SIZE, stdin );
		if( debug_mode ){ printf( "入力文字列：「%s」\n", input_buffer ); }						///DBG

		// 文字切り出し
		p = input_buffer;
		for( i=0; i<NUM_OF_CHAR; i++ ){
			if( *p == '\n' ){
				printf( "Warning : 文字が短すぎます（字足らずで処理します）\n" );
				for( j=i; j<NUM_OF_CHAR; j++ ){
					ku[j] = 0;
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
					ku[i] = j;
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
		if( *p != '\n' ){
			printf( "Warning : 文字が長すぎます（切り捨てて処理します）\n" );
		}

		// 文法チェック
		grammar_check( ku );

		if( debug_mode ){													///DBG
			for( i=0; i<NUM_OF_CHAR; i++ ){									///DBG
				printf( "%d\t%s\n", ku[i], moji[ku[i]] );					///DBG
			}																///DBG
		}																	///DBG

		// 通し番号計算
		mpz_t	mp_number;
		mpz_t	mp_n_size;
		mpz_t	mp_ku;

		mpz_init( mp_number );					// mp_number = 0;
		mpz_init( mp_ku );
		mpz_init( mp_n_size );
		mpz_set_si( mp_n_size, n_size );
		if( debug_mode ){ gmp_printf( "mp_n_size : %Zd\n",  mp_n_size ); }											///DBG

		for( i=NUM_OF_CHAR-1; i>=0; i-- ){
//		for( i=0; i<NUM_OF_CHAR; i++ ){
			if( debug_mode ){ gmp_printf( "pre  mp_number : %Zd,  mp_n_size : %Zd\n", mp_number, mp_n_size ); }		///DBG
			mpz_set_si( mp_ku, ku[i] );
			mpz_mul( mp_number, mp_number, mp_n_size );
			if( debug_mode ){ gmp_printf( "mid  mp_number : %Zd,  mp_ku : %Zd\n", mp_number, mp_ku ); }				///DBG
			mpz_add( mp_number, mp_number, mp_ku );
			if( debug_mode ){ gmp_printf( "post mp_number : %Zd\n", mp_number ); }									///DBG
		}

		// 表示
		printf( "\n" );
		printf( "入力文字列：" );
		for( i=0; i<NUM_OF_CHAR; i++ ){
			printf( "%s", moji[ ku[ i ] ] );
		}
		printf( "\n" );
		mpz_get_str( input_buffer, 10, mp_number );
		convert_num_unit( input_buffer, str_buffer );
		printf( "通し番号　：" );
		gmp_printf( "%Zd\n", mp_number );
		printf( "　　　　　：%s\n", str_buffer );

		// 解放
		mpz_clear( mp_number );
		mpz_clear( mp_ku );
		mpz_clear( mp_n_size );

		// 継続・終了メニュー
		input_num = senryu_end_menu();

	} while( ( input_num == 0 ) && ( *input_buffer != '0' ) );		// 空打ちのとき true、継続。　本当に 0 入力時 false。
																	// ★ この部分は後で見直す。
}


int senryu_num_to_str_f( void ){
	// 番号入力
	// エラーチェック(?)
	// 剰余の計算による、各文字の決定
	// エラーチェック（こっちでやった方がいい？）
	// 表示
	// 継続・終了メニュー

	int			input_num;
	u_int32_t	i, j;
	bool		str_is_short;

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
		mpz_t	mp_ku;

		mpz_init( mp_number );
		mpz_init( mp_number_bak );
		mpz_init( mp_n_size );
		mpz_init( mp_ku );
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
		str_is_short = false;
		for( i=0; i<NUM_OF_CHAR; i++ ){
			mpz_mod( mp_ku, mp_number, mp_n_size );
			mpz_tdiv_q( mp_number, mp_number, mp_n_size );
			ku[i] = mpz_get_ui( mp_ku );
			if( debug_mode ){ gmp_printf( "post mp_number : %3Zd, mp_ku : %Zd, moji[ku[mp_ku]] : %s\n",				///DBG
																	mp_ku, mp_number, moji[ku[i]] ); }				///DBG
			if( ( mpz_get_ui( mp_number ) == 0 ) && ( ku[i] == 0 ) ){
				str_is_short = true;
//				printf( "Warning : 文字が短すぎます\n" );
				for( j=i+1; j<NUM_OF_CHAR; j++ ){
					ku[j] = 0;
				}
				break;
			}
		}

		// 表示
		mpz_get_str( input_buffer, 10, mp_number_bak );
		convert_num_unit( input_buffer, str_buffer );
		printf( "\n" );
		printf( "通し番号　：" );
		gmp_printf( "%Zd\n", mp_number_bak );
		printf( "　　　　　：%s\n", str_buffer );
		printf( "文字列　　：" );
		for( i=0; i<NUM_OF_CHAR; i++ ){
			printf( "%s", moji[ ku[ i ] ] );
		}
		printf( "\n" );

		// 文字長チェック
		if( str_is_short ){
			printf( "Warning : 文字が短すぎます\n" );
		}
		// 文法チェック
		grammar_check( ku );

		// 解放
		mpz_clear( mp_number );
		mpz_clear( mp_ku );
		mpz_clear( mp_n_size );

		// 継続・終了メニュー
		input_num = senryu_end_menu();

	} while( ( input_num == 0 ) && ( *input_buffer != '0' ) );		// 空打ちのとき true、継続。　本当に 0 入力時 false。
}


void senryu_help_f( void ){
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
		mode = senryu_start_menu();
		switch( mode ){
		case SENRYU_MODE_S2N:
			senryu_str_to_num_f();
			break;
		case SENRYU_MODE_N2S:
			senryu_num_to_str_f();
			break;
		case SENRYU_MODE_HELP:
			senryu_help_f();
			break;
		case SENRYU_MODE_EXIT:
			break;
		default:
			break;
		}
	}while( mode != SENRYU_MODE_EXIT );
	printf( "さようなら\n");
	return 0;
}
