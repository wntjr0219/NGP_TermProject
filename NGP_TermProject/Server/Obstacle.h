#pragma once
#include "protocol.h"
#include <Windows.h>
#include <math.h>


extern POSXYZ playerPos;


class Obstacle {
public:
	POSXYZ mPos;
private:
	float mHalfwidth;
	float mY;
public:

	Obstacle() {}

	Obstacle(float r, float y, int h)  {
		mHalfwidth = r;
		mY = y;
		mPos.posX = (float)(rand() % 7);
		mPos.posY = (float)(rand() % h);
		mPos.posZ = -(float)(rand() % (50 * h + 40));
	}
	Obstacle(float r, float y, int h, int z) {
		mHalfwidth = r;
		mY = y;
		mPos.posX = (float)(rand() % 7);
		mPos.posY = (float)(rand() % h);
		mPos.posZ = -(float)(rand() % (50 * h + z));
	}

	bool collide() {
		if (fabs(mPos.posX - 3 - playerPos.posX) > mHalfwidth + 0.49) return false;
		//if (fabs(mPos.posY - playerPos.posY) > 0.49 + mY) return false;
		if (fabs(mPos.posZ - playerPos.posZ) > 0.99) return false;
		if (mPos.posY + mY < playerPos.posY - 0.49) return false;
		if (mPos.posY > playerPos.posY + 0.49) return false;
		return true;
	}

	void reSetObstacle(int i) {

		if(mPos.posZ >= 6.0) {
			if(i < 15) {
				mPos.posX = (float)(rand() % 7);
				mPos.posY = (float)(rand() % 2);
				mPos.posZ = -50.0;

			}
			else {
				mPos.posY = (float)(rand() % 3);
				mPos.posZ = -(float)(rand() % 200 + 40);
			}
			
		}
	}
	void move(int i, int meter) {
		if (i < 5) { mPos.posZ += 0.5; }
		else if ( 5 <= i && i < 10 && meter >= 200) { mPos.posZ += 1.0; }
		else if ( 10 <= i && i < 15 && meter >= 600) { mPos.posZ += 1.0; }
		else if(meter >= 1000 ){ mPos.posZ += 1.15; }
	}
};
