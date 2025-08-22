#include "RulesProcessor.h"
#include "DeviceStateManager.h"
#include "WeatherData.h"
#include "RuleSet.h"

namespace Automation
{

bool RulesProcessor::evaluateRule(const Rule& rule, const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history)
{
	bool all_conditions_met = true;
	for (const auto& condition : rule.conditions)
	{
		if (!condition)
		{
			all_conditions_met = false;
			qWarning() << "RulesProcessor: Empty condition found " << rule.id;
			break;
		}

		if (!condition->evaluate(weather_history, indoor_history))
		{
			// If on condition is not met, the whole rule fails
			all_conditions_met = false;
			break;
		}
	} // Loop over conditions

	return all_conditions_met;
}

/*
* @throws std::runtime_error if at least one device state could not be determined
*/
Device::DeviceStates RulesProcessor::calculateDeviceStates(
	const RuleSet& rule_set,
	std::vector<QString> device_ids,
	const std::vector<WeatherData>& weather_history,
	const std::vector<IndoorData>& indoor_history)
{
	Device::DeviceStates calculated_states;

	// Set all to unknown by default
	for (const auto& device_id : device_ids)
		calculated_states.states.push_back({ device_id, Device::DevicePosition::Unknown });

	// Iterate over rules, that are sorted by priority
	for (const auto& rule : rule_set.getRules())
	{
		// If state already set -> skip (lower prio cant override already set state)
		if (calculated_states.getDevicePosition(rule.device_id) != Device::DevicePosition::Unknown)
			continue;

		if (evaluateRule(rule, weather_history, indoor_history))
			calculated_states.setDevicePosition(rule.device_id, rule.position);

	} // Loop over rules

	return calculated_states;
}
}