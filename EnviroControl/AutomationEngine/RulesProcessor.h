#pragma once

#include "RuleSet.h"
#include <vector>

class QString;

namespace Device
{
class DeviceStates;
}

namespace Automation
{
class Rule;
class RuleSet;
}

struct WeatherData;
struct IndoorData;

namespace Automation
{

class RulesProcessor
{
public:
	static bool evaluateRule(const Rule& rule, const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history);
	static Device::DeviceStates calculateDeviceStates(const RuleSet& rule_set, std::vector<QString> device_ids, const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history);
};
}