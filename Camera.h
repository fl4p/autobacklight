#pragma once

#include <vector>

class Camera
{
public:	
    explicit Camera();
    ~Camera();
	
	float getBrightness();
  
private:
	static int numDevices;
	static std::vector<int> imageData();
	int deviceNo;
};

