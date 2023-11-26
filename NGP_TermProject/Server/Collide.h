#pragma once
#include "protocol.h"
#include <Windows.h>
#include <math.h>


extern POSXYZ playerPos;


class Obastacle {
private:
	POSXYZ mPos;
	float mHalfwidth;
	float mY;
public:

	Obastacle(float r, float y, int h) noexcept {
		mHalfwidth = r;
		mY = y;
		mPos.posX = (float)(rand() % 7);
		mPos.posY = (float)(rand() % h);
		mPos.posZ = (float)(rand() % (50 * h + 40));
	}
	Obastacle(float r, float y, int h, int z) noexcept {
		mHalfwidth = r;
		mY = y;
		mPos.posX = (float)(rand() % 7);
		mPos.posY = (float)(rand() % h);
		mPos.posZ = (float)(rand() % (50 * h + z));
	}

	bool collide() {
		if (fabs(mPos.posX - 3 - playerPos.posX) > mHalfwidth + 0.49) return false;
		//if (fabs(mPos.posY - playerPos.posY) > 0.49 + mY) return false;
		if (fabs(mPos.posZ - playerPos.posZ) > 0.99) return false;
		if (mPos.posY + mY < playerPos.posY - 0.49) return false;
		if (mPos.posY > playerPos.posY + 0.49) return false;
	}
};
