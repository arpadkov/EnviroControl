#include "gtest/gtest.h"

#include "RulesProcessor.h"
#include "DeviceStateManager.h"

#include "WeatherDataCreator.h"

using namespace Automation;

Rule createRuleWithoutConditionWithId(const QString& device_id, int priority, Device::DevicePosition action)
{
	Rule rule;
	rule.id = "id";
	rule.device_id = device_id;
	rule.priority = priority;
	rule.position = action;
	return rule;
}

std::vector<WeatherData> createWindyForLast3Minutes()
{
	std::vector<WeatherData> weather_history;

	auto now = QDateTime::currentDateTime();
	weather_history.push_back(WeatherDataCreator::createWindy(now, 12));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 1), 19));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 2), 18));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 3), 17.5));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 4), 26));

	return weather_history;
}

std::vector<IndoorData> createHotIndoor()
{
	std::vector<IndoorData> indoor_history;

	auto now = QDateTime::currentDateTime();
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now, 22));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 1), 23));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 2), 21));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 3), 26));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 4), 24));

	return indoor_history;
}

TEST(CalculateDeviceStateTest, TestDeviceClosedHighWind)
{
	// Create device (sunblind)
	QString device_id = "sunblind_1";
	std::vector<QString> device_ids = { device_id };

	const auto& weather_history = createWindyForLast3Minutes();
	const auto& indoor_history = createHotIndoor();

	// Create rules
	// 1. Close sunblinds, when high wind (>25)
	auto rule_close_on_high = createRuleWithoutConditionWithId(device_id, 999, Device::DevicePosition::Closed);
	rule_close_on_high.conditions.push_back(std::make_unique<NumericThresholdCondition>(SensorDataSource::WeatherData, "wind_speed", ConditionOperator::GreaterThan, 25));

	// Rule1 should evaluate to false, since currently there is no high wind -> No closed action
	EXPECT_FALSE(RulesProcessor::evaluateRule(rule_close_on_high, weather_history, indoor_history));

	// 2. Open Sunblinds, when:
	//	2.1: Low wind (<20) for the past 3 minutes
	//  2.2: High sunlight
	//  2.3: High IndoorTemp
	//  2.4: No rain
	auto rule_open_sunblind = createRuleWithoutConditionWithId(device_id, 500, Device::DevicePosition::Open);
	rule_open_sunblind.conditions.push_back(std::make_unique<NumericTimeDurationCondition>(SensorDataSource::WeatherData, "wind_speed", ConditionOperator::LessThan, 20, 3 * 60));
	rule_open_sunblind.conditions.push_back(std::make_unique<NumericThresholdCondition>(SensorDataSource::WeatherData, "daylight", ConditionOperator::GreaterThan, 80));
	rule_open_sunblind.conditions.push_back(std::make_unique<NumericThresholdCondition>(SensorDataSource::IndoorData, "indoor_temp", ConditionOperator::GreaterThan, 20));
	rule_open_sunblind.conditions.push_back(std::make_unique<BooleanStateCondition>(SensorDataSource::WeatherData, "is_raining", false)); // Open if NOT raining

	// Rule2 should evaluate to true, since there was no high wind, is light, is hot, no rain -> Open action
	EXPECT_TRUE(RulesProcessor::evaluateRule(rule_open_sunblind, weather_history, indoor_history));

	std::vector<Rule> rules;
	rules.push_back(std::move(rule_close_on_high));
	rules.push_back(std::move(rule_open_sunblind));

	RuleSet rule_set;
	rule_set.setRules(std::move(rules));

	const auto& device_states = RulesProcessor::calculateDeviceStates(rule_set, device_ids, weather_history, indoor_history);
	auto sunblind_state = device_states.getDevicePosition(device_id);

	EXPECT_TRUE(sunblind_state == Device::DevicePosition::Open);

}