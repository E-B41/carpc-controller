/*
 * tda.h
 *
 *  Created on: 30.07.2015
 *      Author: edi
 */

#ifndef TDA_H_
#define TDA_H_

namespace tda {

class tda {
private:
	int address;
	int volume;
	int bass;
	int treble;
	int speakers[4];
	int fade;
	int balance;
	int setAtt(int unit, int dir);
	int i2c_fd;
	int sendI2C(int code);

public:
	tda();
	virtual ~tda();
	int init(int address);
	int getAddress();
	int getVolume();
	float setVolume(int dir);
	int setBass(int dir);
	int setTreble(int dir);
	int setFade(int dir);
	int setBalance(int dir);
	int setInput(int input);
	float initVolume(int value);
	int initBass(int value);
	int initTreble(int value);
	int initFade(int value);
	int initBalance(int value);
	
	

};


}
#endif /* TDA_H_ */
