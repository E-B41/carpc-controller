/*
 * tda.cpp
 *
 *  Created on: 30.07.2015
 *      Author: edi
 */

#include "tda.h"
#include <iostream>
#include <linux/i2c-dev.h>
#include <fstream>
#include <string>
//#include <cstring>
#include <sstream>

#include "wiringPi.h"
#include "wiringPiI2C.h"

static int trebarray[16] = {112,113,114,115,116,117,118,127,126,125,124,123,122,121,120};
static int bassarray[16] = {96,97,98,99,100,101,102,103,110,109,108,107,106,105,104};
static int inputarray[4] = {80,81,82,83}; //+6dB Values
static int speakerarray[4] = {128,160,192,224}; //FL,FR,RL,RR

namespace tda {
tda::tda() {}

tda::~tda() {

}

int tda::init(int address)
{
	std::cout << "TDA Initialization at:  " << address << "\n";
	this->address=address;

	std::ifstream file_settings;
	file_settings.open("/home/pi/.kodi/userdata/addon_data/script.program.tdasettings/settings.xml");

	//settings order: balance, bass, fade, treble, volume
	int j = 0;
	std::string line;
	if (file_settings.is_open())
	{
    	while ( std::getline (file_settings,line) )
    	{

    		if( line != "<settings>" && line != "</settings>")
    		{

    			std::istringstream iss(line);
    			std::string token;
    			for(int i=0;i<4;i++)
    			{
    				std::getline(iss,token, '\"');
    			}
    			int tmp;
    			tmp = std::stoi(token);
    			switch(j)
    			{
    				case 0:
    					tda::initBalance(tmp);
    					//std::cout << "Balance Int: " << tmp << std::endl;
    					break;

    				case 1:
    					tda::initBass(tmp);
    					//std::cout << "Bass Int: " << tmp << std::endl;
    					break;

    				case 2:
						tda::initFade(tmp);
    					//std::cout << "Fade Int: " << tmp << std::endl;
    					break;

    				case 3:
    					tda::initTreble(tmp);
    					//std::cout << "Treble Int: " << tmp << std::endl;
    					break;

    				case 4:
    					tda::initVolume(tmp);
    					//std::cout << "Volume Int: " << tmp << std::endl;
    					break;
    			}
    			j++;
    		}
    	}
    file_settings.close();
    std::cout << "Volume" << this->volume << std::endl;
	}

	return 1;
}

int tda::sendI2C(int code)
{
	//int fd = wiringPiI2CSetup(68);
    //wiringPiI2CWrite(fd, 81);
	this->i2c_fd = wiringPiI2CSetup(this->address);
	int err = wiringPiI2CWrite(this->i2c_fd, code);
	return err;
}

int tda::getAddress()
{
	return this->address;
}

int tda::getVolume()
{
	return this->volume;
}

float tda::initVolume(int value)
{
	sendI2C(value);
	this->volume = value;
}
	
int tda::initBass(int value)
{
	sendI2C(bassarray[value]);
	this->bass = value;
}
	
int tda::initTreble(int value)
{
	sendI2C(trebarray[value]);
	this->treble = value;
}
	
int tda::initFade(int value)
{
	if(value < 0)
	{
		while(this->fade > value)
		{
			tda::setFade(-1);
		}
	}
	else if(value > 0)
	{
		while(this->fade < value)
		{
			tda::setFade(1);
		}
	}
}
	
int tda::initBalance(int value)
{
	if(value < 0)
	{
		while(this->balance > value)
		{
			tda::setBalance(-1);
		}
	}
	else if(value > 0)
	{
		while(this->balance < value)
		{
			tda::setBalance(1);
		}
	}
}





float tda::setVolume(int dir)
{
	if(dir == 1 and this->volume > 0)
	{
		this->volume = this->volume - 1;
		sendI2C(this->volume);
	}
	else if(dir == -1 and this->volume < 63)
	{
		this->volume = this->volume + 1;
		sendI2C(this->volume);
	}
	return -1*volume*1.25;
}


int tda::setBass(int dir)
{
	if(dir == -1 and this->bass > 0)
	{
		this->bass = this->bass - 1;
	}
	if(dir == 1 and this-> bass < 14)
	{
		this->bass = this->bass + 1;
	}
	sendI2C(bassarray[this->bass]);
	return this->bass;
}

int tda::setTreble(int dir)
{
	if(dir == -1 and this->treble > 0)
	{
		this->treble = this->treble - 1;
	}
	if(dir == 1 and this-> treble < 14)
	{
		this->treble = this->treble + 1;
	}
	sendI2C(trebarray[this->treble]);
	return this->treble;
}

int tda::setInput(int input)
{
	if(input < 1 or input > 4) return 0;


	sendI2C(inputarray[input-1]);

	return input;
}

int tda::setAtt(int unit, int dir)
{
	if(dir == -1)
	{
		if(this->speakers[unit] < 31)
			this->speakers[unit] = this->speakers[unit] + 1;
	}
	if(dir == 1)
	{
		if(this->speakers[unit] > 0)
			this->speakers[unit] = this->speakers[unit] - 1;
	}
	int code = speakerarray[unit] + this->speakers[unit];

	sendI2C(code);

	return code;
}

int tda::setFade(int dir)
{
	if(dir == -1) //to back
	{
		if(this->fade > -31 and this->fade <= 0) //even or front reduced
		{
			tda::setAtt(0,-1); //turn down FL
			tda::setAtt(1,-1); //turn down RR
			this->fade = this->fade - 1;
		}
		if(this->fade > 0 and this->fade <= 31) //rear reduced
		{
			tda::setAtt(2,1); //bring up RL
			tda::setAtt(3,1); //bring up RR
			this->fade = this->fade - 1;
		}
		
	}
	if(dir == 1) //to front
	{
		
		if(this->fade > 0 and this->fade < 31) //rear reduced
		{
			tda::setAtt(2,-1); //turn down RL
			tda::setAtt(3,-1); //turn down RR
			this->fade = this->fade + 1;
		}
		if(this->fade >= -31 and this->fade <= 0) //even or front reduced
		{
			tda::setAtt(0,1); //bring up FL
			tda::setAtt(1,1); //bring up FR
			this->fade = this->fade + 1;
		}
		

	}
	return this->fade;
}

int tda::setBalance(int dir)
{
	if(dir == -1) //left
	{
		if(this->balance > -31 and this->balance <= 0) //even or right reduced
		{
			tda::setAtt(1,-1); //turn down FR
			tda::setAtt(3,-1); //turn down RR
			this->balance = this->balance - 1;
		}
		if(this->balance > 0 and this->balance <= 31) //left reduced
		{
			tda::setAtt(0,1); //bring up FL
			tda::setAtt(2,1); //bring up RL
			this->balance = this->balance - 1;
		}
	}
	if(dir == 1) //right
	{
		if(this->balance >= -31 and this->balance <= 0) //even or right reduced
		{
			tda::setAtt(1,1); //bring up FR
			tda::setAtt(3,1); //bring up RR
			this->balance = this->balance + 1;
		}
		if(this->balance > 0 and this->balance <31) //left reduced
		{
			tda::setAtt(0,-1); //turn down FL
			tda::setAtt(2,-1); //turn down RL
			this->balance = this->balance + 1;
		}
	}

	return this->balance;
}

}
