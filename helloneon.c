/*
 * Written by Hwajeong Seo
 * 2015-11-29
 * Montgomery Multiplication 512-bit
 */

#include <android/log.h>

#include <jni.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <cpu-features.h>

#include "kara_mul.h"
#include "mont512.h"
#include "mont1024.h"
#include "kara_sqr.h"

#define DEBUG 1

#if DEBUG
#include <android/log.h>
#  define  D(x...)  __android_log_print(ANDROID_LOG_INFO,"helloneon",x)
#else
#  define  D(...)  do {} while (0)
#endif

/* return current time in milliseconds */
static double now_ms(void) {
	struct timespec res;
	clock_gettime(CLOCK_REALTIME, &res);
	return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//1024-bit parameters work

//a: 0xF57ACC12919B02AFC971EEE19B2368A353545C364779C76A429B5D442617D2304FBD21A9DBAF8C782B202C721F08F8215241A0A49286D03C67D1D37A5392AC1FB47D7102879F29D5CED6A6CBA73F5AF2E15BC400A69A282551A0A9D72D182A818C42A95AD82C6A8E46D7531EEE23EB83D9C6014F17F4290D7B8A4D77080D0136
//uint32_t a1024[32]={0x80D0136,0x7B8A4D77,0x17F4290D,0xD9C6014F,0xEE23EB83,0x46D7531E,0xD82C6A8E,0x8C42A95A,0x2D182A81,0x51A0A9D7,0xA69A2825,0xE15BC400,0xA73F5AF2,0xCED6A6CB,0x879F29D5,0xB47D7102,0x5392AC1F,0x67D1D37A,0x9286D03C,0x5241A0A4,0x1F08F821,0x2B202C72,0xDBAF8C78,0x4FBD21A9,0x2617D230,0x429B5D44,0x4779C76A,0x53545C36,0x9B2368A3,0xC971EEE1,0x919B02AF,0xF57ACC12};

//b: 0x9A7838352A3ED526176405BE4946D9D2FF72FF670E99734EDBD806AD07F0874DFB6F5A32A0EC55DA1F9CE4FB28775564499DC56CBBC43F49033B8FC47F89E9DAD32B558B2C168C8A74B9828F4ADEC7CC146FF0DB0FDC49D56E1F8DC90E56D91FC004500C36607BBBC2705055C8C8F85E4CB3BA8BC42CFD46BAA82A38CC529620
//uint32_t b1024[32]={0xCC529620,0xBAA82A38,0xC42CFD46,0x4CB3BA8B,0xC8C8F85E,0xC2705055,0x36607BBB,0xC004500C,0xE56D91F,0x6E1F8DC9,0xFDC49D5,0x146FF0DB,0x4ADEC7CC,0x74B9828F,0x2C168C8A,0xD32B558B,0x7F89E9DA,0x33B8FC4,0xBBC43F49,0x499DC56C,0x28775564,0x1F9CE4FB,0xA0EC55DA,0xFB6F5A32,0x7F0874D,0xDBD806AD,0xE99734E,0xFF72FF67,0x4946D9D2,0x176405BE,0x2A3ED526,0x9A783835};
uint32_t a1024[32] = { 0x1234567, 0x89ABDCEF, 0x1234567, 0x89ABDCEF, 0x1234567,
		0x89ABDCEF, 0x1234567, 0x89ABDCEF, 0x1234567, 0x89ABDCEF, 0x1234567,
		0x89ABDCEF, 0x1234567, 0x89ABDCEF, 0x1234567, 0x89ABDCEF, 0x1234567,
		0x89ABDCEF, 0x1234567, 0x89ABDCEF, 0x1234567, 0x89ABDCEF, 0x1234567,
		0x89ABDCEF, 0x1234567, 0x89ABDCEF, 0x1234567, 0x89ABDCEF, 0x1234567,
		0x89ABDCEF, 0x1234567, 0x89ABDCEF };

uint32_t b1024[32] = { 0xC50B1E5A, 0x96DE039D, 0x6DECCB92, 0x91E29DD8,
		0x3437FCB2, 0xE5B4546D, 0xBD131CD0, 0xF3670BC3, 0x602DA9DA, 0x5343297B,
		0x15E8FE80, 0xA15CC6D3, 0xA38B21FE, 0x48A4D7EB, 0xB38609E5, 0x7ED5E07E,
		0x55ABCB6F, 0x96959475, 0x47DA5122, 0x89DF0CBE, 0x8D577CCD, 0x1D72AEE9,
		0x59D0D2F, 0x5B3626C7, 0xBF626F29, 0x48562E4A, 0x1A0BFDEA, 0xF2AFAA4,
		0x77097C0C, 0xED90A015, 0xDB631662, 0x49A544B6 };

uint32_t rr1024[64] = { 0, };
uint32_t rr1024_2[64] = { 0, };

//m:=0x8EBF031AF6BA36B71B2A4DC3D81CE87CE181AD28406C4C657B409704661F59B3231C7819C2CC86CE338D46A8923C5188C0A1A6DA84F4B658AA7F68CAAE895E1377CD49EF95E03B533624BC3D85F3A9B20B3542543072D447A7D16BFA76EBF2F129B5AC1F392C1C4F00E37AD40B20C8FF49C4A23D5306D4CBAF967DAFAF6677B5
uint32_t m1024[4 + 32 + 32 + 32] = {
		//m_inverse, m_inverse, 1, 1
		0xF2480163, 0xF2480163, 0x00000001,
		0x00000001
		//m 1  5  2  6  3  7  4  8
		//m 9 13 10 14 11 15 12 16
		//m17 21 18 22 19 23 20 24
		//m25 29 26 30 27 31 28 32
		, 0xAF6677B5, 0xB20C8FF, 0xAF967DAF, 0xE37AD4, 0x5306D4CB, 0x392C1C4F,
		0x49C4A23D, 0x29B5AC1F, 0x76EBF2F1, 0x85F3A9B2, 0xA7D16BFA, 0x3624BC3D,
		0x3072D447, 0x95E03B53, 0xB354254, 0x77CD49EF, 0xAE895E13, 0x923C5188,
		0xAA7F68CA, 0x338D46A8, 0x84F4B658, 0xC2CC86CE, 0xC0A1A6DA, 0x231C7819,
		0x661F59B3, 0xD81CE87C, 0x7B409704, 0x1B2A4DC3, 0x406C4C65, 0xF6BA36B7,
		0xE181AD28,
		0x8EBF031A
		//m 1  2  3  4  5  6  7  8
		//m 9 10 11 12 13 14 15 16
		//m17 18 19 20 21 22 23 24
		//m25 26 27 28 29 30 31 32
		, 0xAF6677B5, 0xAF967DAF, 0x5306D4CB, 0x49C4A23D, 0xB20C8FF, 0xE37AD4,
		0x392C1C4F, 0x29B5AC1F, 0x76EBF2F1, 0xA7D16BFA, 0x3072D447, 0xB354254,
		0x85F3A9B2, 0x3624BC3D, 0x95E03B53, 0x77CD49EF, 0xAE895E13, 0xAA7F68CA,
		0x84F4B658, 0xC0A1A6DA, 0x923C5188, 0x338D46A8, 0xC2CC86CE, 0x231C7819,
		0x661F59B3, 0x7B409704, 0x406C4C65, 0xE181AD28, 0xD81CE87C, 0x1B2A4DC3,
		0xF6BA36B7,
		0x8EBF031A
		//idle
		, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0 };
/**/
jstring Java_com_example_neon_HelloNeon_stringFromJNI(JNIEnv* env, jobject thiz) {
	char* str;
	uint64_t features;
	char buffer[500];
	char tryNeon = 0;
	// double  t0, t1, time_neon;

#ifdef HAVE_NEON

	int count = 0;
	//t0 = now_ms();
	//rsa_demo_1024();

	//MUL_2048(r2048,a2048,b2048);
	//MUL_1024(rr1024,a1024,b1024);
	//SQR_512(r,a);
	//MM_512(r2,r,m);
	//SQR_1024(r1024,a1024);
	//MM_1024(r1024_2,r1024,m1024);
	rsa_demo_1024();

	//t1 = now_ms();
	//time_neon = t1 - t0;
/*
	for(count=0;count<64;count++) {
		D("%d: 0x%x", count, rr1024[count]);
		//D("%d: 0x%x", count, r2[count]);
	}/**/

	free(str);

#else /* !HAVE_NEON */
	strlcat(buffer, "Program not compiled with ARMv7 support !\n",
			sizeof buffer);
#endif /* !HAVE_NEON */
	EXIT: return (*env)->NewStringUTF(env, buffer);
}
