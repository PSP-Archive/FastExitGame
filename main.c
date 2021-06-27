/*
FastExitGame


協力してくださったhiroi01さん SnyFbSxさん plum_429さん Dadrfyさん
ありがとうございました。

5.03の識別は
http://addpadding.seesaa.net/article/185891572.html
のDavee氏のHENのソースコードを利用させてもらいました。
Davee氏、Dadrfyさん　ありがとうございます。

*/

#include <pspkernel.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspinit.h>
#include <kubridge.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

#include <stdio.h>
#include <string.h>
#include "libinip.h"

/*-----------------------------------------------------------------*/

PSP_MODULE_INFO("FastExitGame", 0x1000, 2, 4);

/*------------------------------------------------------------------*/

#define EBOOTBIN "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"
#define INI_PATH "/FastExitGame.ini"

/*------------------------------------------------------------------*/

// キー用変数 iniの項目分必要
ILP_Key key[5];

u32 button[4] = {
	PSP_CTRL_RTRIGGER,
	PSP_CTRL_LTRIGGER,
	PSP_CTRL_TRIANGLE,
	PSP_CTRL_SQUARE
};
/*
Rebootkey
Shutdownkey
Sleepkey
Resetkey
*/
/*------------------------------------------------------------------*/

int scePowerRequestColdReset(int);//reboot

/*------------------------------------------------------------------*/
void roadIni(char *path)
{
	u32 Bootkey;
	char iniPath[256];
	char *temp;
	int i;
	SceIoStat stat;

	strcpy(iniPath, path);
	temp = strrchr(iniPath, '/');
	if( temp != NULL ) *temp = '\0';
	strcat(iniPath, INI_PATH);

	if(!( sceIoGetstat(iniPath, &stat) < 0 ))
	{
		memset(&button, 0, sizeof(button));
	}
	
	// iniファイルの読み込みの下準備
	ILPInitKey(key);

	ILPRegisterButton(key,	"Bootkey", 		&Bootkey,	PSP_CTRL_HOME, NULL);
	ILPRegisterButton(key,	"Rebootkey",	&button[0],	button[0],	NULL);
	ILPRegisterButton(key,	"Shutdownkey",	&button[1],	button[1],	NULL);
	ILPRegisterButton(key,	"Sleepkey",		&button[2],	button[2],	NULL);
	ILPRegisterButton(key,	"Resetkey",		&button[3],	button[3],	NULL);

	if(ILPReadFromFile(key, iniPath) == -1){
		// iniファイルへ書き込み
		ILPWriteToFile(key, iniPath, "\r\n");//読み込めなかったら自動で作成
	}
	
	for(i=0; i<4; i++)
	{
		if( button[i] ) button[i] |= Bootkey;
	}
}

int LoadExecVSH(int apitype, char *path)
{
	struct SceKernelLoadExecVSHParam param;

	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);

	switch(apitype)
	{
		case 0x141: //homebrew mode
		case 0x152: //homebrew PSPGo
			param.args = strlen(path)+1;
			param.argp = path;
			param.key = "game";
			break;
		case 0x120: //ISO mode
		case 0x123:
		case 0x125: //ISO PSPGo
			param.args = strlen(EBOOTBIN)+1;
			param.argp = EBOOTBIN;
			param.key = (apitype == 0x120 ? "game" : "umdemu");
			break;
		case 0x144: // POPS mode
		case 0x155: // POPS PSPGo
			param.args = strlen(path)+1;
			param.argp = path;
			param.key = "pops";
			break;
	}
	sctrlKernelLoadExecVSHWithApitype( apitype , path , &param);

	return 0;
}

//メイン
int main_thread(SceSize args, void *argp)
{

	SceCtrlData pad;
	int apitype;
	int i;
	char gamepath[256];

	while(sceKernelFindModuleByName("sceKernelLibrary") == NULL)
	{
		sceKernelDelayThread(1000);
	}

	roadIni(argp);
	
	apitype = kuKernelInitApitype();
	kuKernelInitFileName(gamepath);
	
	if(apitype==0x123 || apitype==0x125)
	{
		strcpy(gamepath, sctrlSEGetUmdFile());
	}

	while (1)
	{
		sceKernelDelayThread(50000);
		sceCtrlPeekBufferPositive(&pad, 1);
		
		for(i=0; i<4; i++)
		{
			if( (button[i]) && ((pad.Buttons & button[i]) == button[i]) )
			{
				switch(i)
				{
					case 0:
						sceKernelExitVSHVSH(0);//restart VSH
						break;
					case 1:
						scePowerRequestStandby();//shutdown
						break;
					case 2:
						scePowerRequestSuspend();//suspend(sleep mode)
						break;
					case 3:
						if(kuKernelInitKeyConfig() != PSP_INIT_KEYCONFIG_VSH)
							LoadExecVSH(apitype, gamepath);//SoftReset
						break;
				}
			}
		}
	}
	return 0;
}


int module_start(SceSize args, void *argp)
{
	int thid = sceKernelCreateThread("FastExitGame", main_thread, 32, 0x1000, 0, NULL);
	if(thid >= 0)sceKernelStartThread(thid, args, argp);

	return 0;
}


int module_stop(SceSize args, void *argp)
{
	return 0;
}