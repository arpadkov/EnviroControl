#pragma once

#include <vector>

namespace Device
{
class DeviceStates;
}

struct WeatherData;

namespace Automation
{

class RuleSet // Potentially DeviceConfig including rules????
{
};

class RulesProcessor
{
public:
	static Device::DeviceStates calculateDeviceStates(const RuleSet& rule_set, const std::vector<WeatherData>& weather_history);
};
}