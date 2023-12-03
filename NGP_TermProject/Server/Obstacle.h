#pragma once
#include "protocol.h"
#include <Windows.h>
#include <math.h>
#include <random>


extern POSXYZ playerPos;
std::random_device rd;
std::minstd_rand gen(rd());


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
		mPos.posZ = -(float)(rand() % (50 * h) + 40);
	}
	Obstacle(float r, float y, int h, int z) {
		mHalfwidth = r;
		mY = y;
		mPos.posX = (float)(rand() % 7);
		mPos.posY = (float)(rand() % h);
		mPos.posZ = -(float)(rand() % (50 * h) + z);
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
		if (mPos.posZ >= 6.0) {
			if (i < 15) {
				std::uniform_real_distribution<float> posX(0.0f, 6.0f);
				std::uniform_real_distribution<float> posY(0.0f, 2.0f);

				mPos.posX = posX(gen);
				mPos.posY = posY(gen);
				mPos.posZ = -50.0;
			}
			else {
				std::uniform_real_distribution<float> posY(0.0f, 3.0f);
				std::uniform_real_distribution<float> posZ(-240.0f, -40.0f);

				mPos.posY = posY(gen);
				mPos.posZ = posZ(gen);
			}
		}

	}

	void move(int i, int meter) {
		if (i < 5) { mPos.posZ += 0.3; }
		else if ( 5 <= i && i < 10 && meter >= 200) { mPos.posZ += 0.6; }
		else if ( 10 <= i && i < 15 && meter >= 600) { mPos.posZ += 0.6; }
		else if(meter >= 1000 ){ mPos.posZ += 0.69; }
	}
};
