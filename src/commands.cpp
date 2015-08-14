#include <pthread.h>
#include <mqueue.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <sstream>

#include "radio.h"
#include "common.h"
#include "config.h"
#include "si4703.h"
#include "xbmcclient_if.h"
#include "event-server.h"
#include "tda.h"

/*
 * Defines
 */


/*
 * Typedefs
 */

/*
 * Private Functions Declarations
 */
static void Commands_Execute(char *pStr);
static void *Commands_ProcessLoop(void *pParam);

/*
 * Local Variables
 */
static pthread_t        mCommandsProcessThread;
static mqd_t            mCommandsMessageQueueFd;
static struct mq_attr   mCommandsMessageQueueBuf;
static bool_t           mForceStop = FALSE;


/*
 * Global Variables
 */
extern uint8_t          gSystemMode;
extern bool_t           gRdsEnabled;
extern bool_t           gFrequencyChanged;
extern tda::tda         *TDA;
/*
 * Public functions definitions
 */
void Commands_Init()
{
   

    //pthread_mutex_init(&lockX, NULL);
    print("Starting Commands Module\n");

    /* Create message queue */
    mCommandsMessageQueueBuf.mq_msgsize = 512;
    mCommandsMessageQueueBuf.mq_maxmsg = 16;
    mCommandsMessageQueueFd = mq_open("/QCarPC", O_CREAT | O_RDWR, 600, &mCommandsMessageQueueBuf);

    if(mCommandsMessageQueueFd < 0)
    {
        perror("Error");
    }
    else
    {
        print("init ok %d\n", mCommandsMessageQueueFd);

        /* Create radio periodically update thread */
        pthread_create(&mCommandsProcessThread, NULL, Commands_ProcessLoop, NULL);
    }
}

void Commands_UnInit()
{
    mq_close(mCommandsMessageQueueFd);
}

void Commands_Signal(char *pStr)
{
    int res;
    print("sending: %s\n", pStr);

    res = mq_send(mCommandsMessageQueueFd, pStr, strlen(pStr) + 1, 1);
    if(res < 0)
	{
    	print("error fd %d\n", mCommandsMessageQueueFd);
	}
}


/*
 * Private functions definitions
 */
static void *Commands_ProcessLoop(void *pParam)
{
    unsigned int msgPrio;
    void *pBuff = calloc(512, 1);

    while(!mForceStop)
    {
        int ret;

        ret = mq_receive(mCommandsMessageQueueFd, (char*)pBuff, mCommandsMessageQueueBuf.mq_msgsize,
            &msgPrio);
        
        if (ret >= 0) /* got a message */
        {
            Commands_Execute((char*)pBuff);
        }
        else if(ret < 0)
		{
			print("received: %d\n", ret);
		}
    }
    pthread_exit(NULL);
    free(pBuff);

    return NULL;
}

/*! \brief Parse a string and send the commands found via sockets.
 *
 *  \param[IN/OUT] pStr pointer to the command that should be executed
 */
static void Commands_Execute(char *pStr)
{
    char        *pCommands[MAX_COMMANDS];
    uint8_t     iComm = 0;
    char        *pComm = NULL;
    char        *pTmpStr = NULL;
    char        *pOrigStr = NULL;

    //pthread_mutex_lock(&lockX);

    if(pStr)
    {
    	printf("processing [%s]\n", pStr);
    	
        //print("System mode: %d\n", gSystemMode);

    	memset(pCommands, 0, MAX_COMMANDS * sizeof(pCommands[0]));


    	/* Get the number of commands and setup pCommands array */
    	pTmpStr = strdup(pStr);
    	pOrigStr = pTmpStr; /* Used to free the correct buffer */
    	iComm = 0;
    	pCommands[iComm] = pTmpStr;
    	while(*pTmpStr)
    	{
    		if(*pTmpStr == '+')
    		{
    			iComm++;
    			pCommands[iComm] = pTmpStr + 1;
    			*pTmpStr = '\0';
    		}

    		/* Sanity check */
    		if(iComm >= MAX_COMMANDS - 1) break;

    		pTmpStr++;
    	}
    	iComm++;

        
    	/* Iterate through commands */
    	while(iComm)
    	{
    		pComm = pCommands[iComm - 1];

    		if(!strcmp(pComm, "interrupt_rds") && (gSystemMode == gModeRadio_c))
    		{
    		    Radio_RdsUpdate();
    		}
    		/*TDA Commands */
            else if(strstr(pComm, TDA_CMD))
            {
                printf("TDA Command:  ");

                if(strstr(pComm,TDA_INIT))
                {
                    TDA->initVolume(0);
                    TDA->initBass(0);
                    TDA->initTreble(0);
                    TDA->initFade(0);
                    TDA->initBalance(0);
                }

                //Volume
                else if(strstr(pComm, TDA_VOL_UP))
                {
                    printf("Volume up to: ");
                    float out = TDA->setVolume(1);
                    printf("%f \n", out);
                    std::stringstream ss_cmd;
                    ss_cmd << "SetProperty(Volume.Value," << TDA->getVolume() <<",10000)";
                    const std::string& ss_str = ss_cmd.str();
                    const char* ss_cstr = ss_str.c_str();
                    XBMC_ClientAction(ss_cstr);
                    XBMC_ClientAction("SetProperty(Volume.Changed,true,10000)");
                }
                else if(strstr(pComm,TDA_VOL_DOWN))
                {
                    printf("Volume down to: ");
                    float out = TDA->setVolume(-1);
                    printf("%f \n", out);
                    std::stringstream ss_cmd;
                    ss_cmd << "SetProperty(Volume.Value," << TDA->getVolume() <<",10000)";
                    const std::string& ss_str = ss_cmd.str();
                    const char* ss_cstr = ss_str.c_str();
                    XBMC_ClientAction(ss_cstr);
                    XBMC_ClientAction("SetProperty(Volume.Changed,true,10000)");
                }
                //Bass
                else if(strstr(pComm,TDA_BA_UP))
                {
                    printf("Bass up to: ");
                    int out = TDA->setBass(1);
                    printf("%i \n", out);
                }
                else if(strstr(pComm,TDA_BA_DOWN))
                {
                    printf("Bass down to: ");
                    int out = TDA->setBass(-1);
                    printf("%i \n", out);
                }
                //Treble
                else if(strstr(pComm,TDA_TR_UP))
                {
                    printf("Treble up to: ");
                    int out = TDA->setTreble(1);
                    printf("%i \n", out);
                }
                else if(strstr(pComm,TDA_TR_DOWN))
                {
                    printf("Treble down to: ");
                    int out = TDA->setTreble(-1);
                    printf("%i \n", out);
                }
                //Fade
                else if(strstr(pComm,TDA_FA_FR))
                {
                    printf("Fade front to: ");
                    int out = TDA->setFade(1);
                    printf("%i \n", out);
                }
                else if(strstr(pComm,TDA_FA_RE))
                {
                    printf("Fade rear to: ");
                    int out = TDA->setFade(-1);
                    printf("%i \n", out);
                }
                //Balance
                else if(strstr(pComm,TDA_BL_LE))
                {
                    printf("Balance left to: ");
                    int out = TDA->setBalance(-1);
                    printf("%i \n", out);
                }
                else if(strstr(pComm,TDA_BL_RE))
                {
                    printf("Balance right to: ");
                    int out = TDA->setBalance(1);
                    printf("%i \n", out);
                }
                //Input
                else if(strstr(pComm,TDA_IN1))
                {
                    printf("Input 1");
                    int out = TDA->setInput(1);
                    printf("%i \n", out);
                }
                else if(strstr(pComm,TDA_IN2))
                {
                    printf("Input 2");
                    int out = TDA->setInput(2);
                    printf("%i \n", out);
                }
                else if(strstr(pComm,TDA_IN3))
                {
                    printf("Input 3");
                    int out = TDA->setInput(3);
                    printf("%i \n", out);
                }
                else if(strstr(pComm,TDA_IN4))
                {
                    printf("Input 4");
                    int out = TDA->setInput(4);
                    printf("%i \n", out);
                }
                

            }
            
            /* XBMC Keyboard action */
    		else if((strstr(pComm, XBMCBUTTON_CMD)) && (gSystemMode == gModeXBMC_c))
    		{
    			XBMC_ClientButton((const char*)pComm + strlen(XBMCBUTTON_CMD), "KB");
    			//print("KB_%s\n", pComm + strlen(XBMCBUTTON_CMD));
    		}
    		/* XBMC built-in function */
    		else if((strstr(pComm, XBMCBUILTIN_CMD)) && (gSystemMode == gModeXBMC_c))
    		{
    			XBMC_ClientAction(pComm + strlen(XBMCBUILTIN_CMD));
    			//print("XBMC builtin_%s\n", pComm + strle(XBMCBUILTIN_CMD));
    		}
    		/* Toggle Radio/Media Center */
    		else if(strstr((const char*)pComm, SYSTEM_MODE_TOGGLE))
    		{
                gSystemMode ^= 0x01;
                
    			if(gSystemMode == gModeRadio_c)
    			{
    				XBMC_ClientAction("PlayerControl(Stop)");
    				XBMC_ClientAction("SetProperty(Radio.Active,true,10000)");
    				int out = TDA->setInput(2);
                    printf("%i", out);
                    si_mute(0);
    			}
    			else
    			{
    				XBMC_ClientAction("SetProperty(Radio.Active,false,10000)");
                    int out = TDA->setInput(1);
                    printf("%i", out);
    				si_mute(1);
    			}
    			print("Mode: %s\n", (gSystemMode==gModeRadio_c)?"radio":"media center");
    		}
    		/* Volume should be updated for both Radio and XBMC */
    		else if(strstr(pComm, SYS_CMD_VOLUME))
    		{
    			System_VolumeUpdate(pComm);
    		}
    		/* RADIO action */
    		else if((strstr(pComm, RADIO_CMD)) && (gSystemMode == gModeRadio_c))
    		{
    			if(!memcmp(pComm, RADIO_CMD_SEEK_UP, strlen(RADIO_CMD_SEEK_UP)))
    			{
    				si_seek(SEEK_UP);
    				gFrequencyChanged = TRUE;
    			}
    			else if(!memcmp(pComm, RADIO_CMD_SEEK_DOWN, strlen(RADIO_CMD_SEEK_DOWN)))
    			{
                    si_seek(SEEK_DOWN);
    				gFrequencyChanged = TRUE;
    			}
    			else if(!memcmp(pComm, RADIO_CMD_TUNE_UP, strlen(RADIO_CMD_TUNE_UP)))
    			{
    				int freq = si_get_freq();

    				if(freq == 10800)
    				{
    					freq = 8800;
    				}
    				else
    				{
    					freq += 10;
    				}

    				si_tune(freq);
    				gFrequencyChanged = TRUE;
    			}
    			else if(!memcmp(pComm, RADIO_CMD_TUNE_DOWN, strlen(RADIO_CMD_TUNE_DOWN)))
    			{
    				int freq = si_get_freq();

    				if(freq == 8800)
    				{
    					freq = 10800;
    				}
    				else
    				{
    					freq -= 10;
    				}

    				si_tune(freq);
    				gFrequencyChanged = TRUE;
    			}
    			else if(strstr((const char*)pComm, RADIO_CMD_TUNE))
    			{
    				int frequency;
    				char *pPos;
    				char *pPos2;

    				pPos = (char*)pComm + strlen(RADIO_CMD_TUNE);

    				pPos2 = strchr(pPos, '.');
    				*pPos2 = *(pPos2 + 1);
    				*(pPos2 + 1) = *(pPos2 + 2);
    				*(pPos2 + 2) = 0;

    				frequency = atoi((const char*)(pComm + strlen(RADIO_CMD_TUNE)));
    				print("new freq: %d\n", frequency);
    				print("frequency to tune: %s\n", (char*)pComm + strlen(RADIO_CMD_TUNE));

    				si_tune(frequency * 10);
    				gFrequencyChanged = TRUE;
    			}
    		}
    		


            /* NAVIGATION commands */
    		/*else if(!strcmp(pComm, NAV_CMD_ZOOM_IN))
    		{
    			system("xdotool keyup '+'");
    		}
    		else if(!strcmp(pComm, NAV_CMD_ZOOM_OUT))
    		{
    			system("xdotool keyup '-'");
    		}
    		else if(!strcmp(pComm, NAV_CMD_UP))
    		{
    			system("xdotool keyup 'up'");
    		}
    		else if(!strcmp(pComm, NAV_CMD_DOWN))
    		{
    			system("xdotool keyup 'down'");
    		}
    		else if(!strcmp(pComm, NAV_CMD_LEFT))
    		{
    			system("xdotool keyup 'left'");
    		}
    		else if(!strcmp(pComm, NAV_CMD_RIGHT))
    		{
    			system("xdotool keyup 'right'");
    		}*/

    		iComm--;
    	}

    	if(gFrequencyChanged)
    	{
    	    Radio_FrequencyUpdate(TRUE);
    	    gFrequencyChanged = FALSE;
    	}

    	free(pOrigStr);
    }
    //pthread_mutex_unlock(&lockX);
}
