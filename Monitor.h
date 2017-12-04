#pragma once

#include <physicalmonitorenumerationapi.h>
#include <highlevelmonitorconfigurationapi.h>

#include <vector>

class Monitor
{
public:
	static std::vector<Monitor> EnumerateMonitors();
    explicit Monitor(PHYSICAL_MONITOR pm);
    ~Monitor();

    bool brightnessSupported() const;

    int minimumBrightness() const;
    int maximumBrightness() const;
    int currentBrightness() const;

    void setCurrentBrightness(int b);
    // Set brightness from 0.0-1.0
    void setCurrentBrightnessFraction(double fraction);

private:
    bool mBrightnessSupported = false;

    int mMinimumBrightness = 0;
    int mMaximumBrightness = 0;
    int mCurrentBrightness = 0;
    PHYSICAL_MONITOR mPhysicalMonitor;
};

