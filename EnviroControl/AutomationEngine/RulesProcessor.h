#pragma once

#include <vector>

class QString;

namespace Device
{
class DeviceStates;
}

namespace Automation
{
class RuleSet;
}

struct WeatherData;
struct IndoorData;

namespace Automation
{

class RulesProcessor
{
public:
	static Device::DeviceStates calculateDeviceStates(const RuleSet& rule_set, std::vector<QString> device_ids, const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history);
};
}