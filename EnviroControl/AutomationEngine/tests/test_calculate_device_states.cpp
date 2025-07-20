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

RuleSet createRuleSetSunblind(const QString& device_id)
{
	// SUNBLIND: OPEN = RETRACTED, CLOSE = EXTENDED

	// Create rules
	// 1. Open sunblinds, when high wind (>25)
	auto rule_open_on_high = createRuleWithoutConditionWithId(device_id, 999, Device::DevicePosition::Open);
	rule_open_on_high.conditions.push_back(std::make_unique<NumericThresholdCondition>(SensorDataSource::WeatherData, "wind_speed", ConditionOperator::GreaterThan, 25));

	// 2. Close Sunblinds, when:
	//	2.1: Low wind (<20) for the past 3 minutes
	//  2.2: High sunlight
	//  2.3: High IndoorTemp
	//  2.4: No rain
	auto rule_close_sunblind = createRuleWithoutConditionWithId(device_id, 500, Device::DevicePosition::Closed);
	rule_close_sunblind.conditions.push_back(std::make_unique<NumericTimeDurationCondition>(SensorDataSource::WeatherData, "wind_speed", ConditionOperator::LessThan, 20, 3 * 60));
	rule_close_sunblind.conditions.push_back(std::make_unique<NumericThresholdCondition>(SensorDataSource::WeatherData, "daylight", ConditionOperator::GreaterThan, 80));
	rule_close_sunblind.conditions.push_back(std::make_unique<NumericThresholdCondition>(SensorDataSource::IndoorData, "indoor_temp", ConditionOperator::GreaterThan, 20));
	rule_close_sunblind.conditions.push_back(std::make_unique<BooleanStateCondition>(SensorDataSource::WeatherData, "is_raining", false)); // Open if NOT raining

	// 3. Default open
	auto rule_default_close_sunblind = createRuleWithoutConditionWithId(device_id, 1, Device::DevicePosition::Open);

	std::vector<Rule> rules;
	rules.push_back(std::move(rule_open_on_high));
	rules.push_back(std::move(rule_close_sunblind));
	rules.push_back(std::move(rule_default_close_sunblind));

	RuleSet rule_set;
	rule_set.setRules(std::move(rules));
	return rule_set;
}

RuleSet createRuleSetWindow(const QString& device_id)
{

	// Create rules
	// 1. Close window, when high wind (>25)
	auto rule_close_on_high = createRuleWithoutConditionWithId(device_id, 999, Device::DevicePosition::Closed);
	rule_close_on_high.conditions.push_back(std::make_unique<NumericThresholdCondition>(SensorDataSource::WeatherData, "wind_speed", ConditionOperator::GreaterThan, 25));

	// 2. Open window, when:
	//  2.3: High IndoorTemp
	//  2.4: No rain
	auto rule_open_window = createRuleWithoutConditionWithId(device_id, 500, Device::DevicePosition::Open);
	rule_open_window.conditions.push_back(std::make_unique<NumericThresholdCondition>(SensorDataSource::IndoorData, "indoor_temp", ConditionOperator::GreaterThan, 20));
	rule_open_window.conditions.push_back(std::make_unique<BooleanStateCondition>(SensorDataSource::WeatherData, "is_raining", false)); // Open if NOT raining

	// 3. Default close
	auto rule_default_close_window = createRuleWithoutConditionWithId(device_id, 1, Device::DevicePosition::Closed);

	std::vector<Rule> rules;
	rules.push_back(std::move(rule_close_on_high));
	rules.push_back(std::move(rule_open_window));
	rules.push_back(std::move(rule_default_close_window));

	RuleSet rule_set;
	rule_set.setRules(std::move(rules));
	return rule_set;
}

TEST(CalculateDeviceStateTest, TestSunblindWasWindClose)
{
	// Create device (sunblind)
	QString device_id = "sunblind_1";
	std::vector<QString> device_ids = { device_id };
	auto now = QDateTime::currentDateTime();

	const auto& rule_set = createRuleSetSunblind(device_id);


	std::vector<WeatherData> weather_history;
	weather_history.push_back(WeatherDataCreator::createWindy(now, 12));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 1), 19));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 2), 18));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 3), 17.5));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 4), 26)); // <- this allows closing, because older than 3 minutes

	std::vector<IndoorData> indoor_history;
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now, 22));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 1), 23));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 2), 21));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 3), 26));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 4), 24));

	const auto& device_states = RulesProcessor::calculateDeviceStates(rule_set, device_ids, weather_history, indoor_history);
	auto sunblind_state = device_states.getDevicePosition(device_id);

	EXPECT_TRUE(sunblind_state == Device::DevicePosition::Closed);
}

TEST(CalculateDeviceStateTest, TestSunblindWasWindOpen)
{
	// Create device (sunblind)
	QString device_id = "sunblind_1";
	std::vector<QString> device_ids = { device_id };
	auto now = QDateTime::currentDateTime();

	const auto& rule_set = createRuleSetSunblind(device_id);

	std::vector<WeatherData> weather_history;
	weather_history.push_back(WeatherDataCreator::createWindy(now, 12));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 1), 21)); // <- this wont allow closing
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 2), 18));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 3), 17.5));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 4), 26));

	std::vector<IndoorData> indoor_history;
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now, 12));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 1), 23));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 2), 21));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 3), 26));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 4), 24));

	const auto& device_states = RulesProcessor::calculateDeviceStates(rule_set, device_ids, weather_history, indoor_history);
	auto sunblind_state = device_states.getDevicePosition(device_id);

	EXPECT_TRUE(sunblind_state == Device::DevicePosition::Open);
}

TEST(CalculateDeviceStateTest, TestSunblindHighWindOpen)
{
	// Create device (sunblind)
	QString device_id = "sunblind_1";
	std::vector<QString> device_ids = { device_id };
	auto now = QDateTime::currentDateTime();

	const auto& rule_set = createRuleSetSunblind(device_id);

	std::vector<WeatherData> weather_history;
	weather_history.push_back(WeatherDataCreator::createWindy(now, 26));

	std::vector<IndoorData> indoor_history;
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now, 22));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 1), 23));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 2), 21));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 3), 26));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 4), 24));

	const auto& device_states = RulesProcessor::calculateDeviceStates(rule_set, device_ids, weather_history, indoor_history);
	auto sunblind_state = device_states.getDevicePosition(device_id);

	EXPECT_TRUE(sunblind_state == Device::DevicePosition::Open);
}

TEST(CalculateDeviceStateTest, TestSunblindLowWindLowIndoorOpen)
{
	// Create device (sunblind)
	QString device_id = "sunblind_1";
	std::vector<QString> device_ids = { device_id };
	auto now = QDateTime::currentDateTime();

	const auto& rule_set = createRuleSetSunblind(device_id);

	std::vector<WeatherData> weather_history;
	weather_history.push_back(WeatherDataCreator::createWindy(now, 12));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 1), 19));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 2), 18));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 3), 17.5));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 4), 26));

	std::vector<IndoorData> indoor_history;
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now, 12));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 1), 11));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 2), 15));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 3), 14));
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now.addSecs(-60 * 4), 11));

	const auto& device_states = RulesProcessor::calculateDeviceStates(rule_set, device_ids, weather_history, indoor_history);
	auto sunblind_state = device_states.getDevicePosition(device_id);

	EXPECT_TRUE(sunblind_state == Device::DevicePosition::Open);
}

TEST(CalculateDeviceStateTest, TestWindowHotIndoorOpen)
{
	// Create device (sunblind)
	QString device_id = "window_1";
	std::vector<QString> device_ids = { device_id };
	auto now = QDateTime::currentDateTime();

	const auto& rule_set = createRuleSetWindow(device_id);

	std::vector<WeatherData> weather_history;
	weather_history.push_back(WeatherDataCreator::createWindy(now, 12));

	std::vector<IndoorData> indoor_history;
	indoor_history.push_back(WeatherDataCreator::createIndoorData(now, 22));

	const auto& device_states = RulesProcessor::calculateDeviceStates(rule_set, device_ids, weather_history, indoor_history);
	auto window_state = device_states.getDevicePosition(device_id);

	EXPECT_TRUE(window_state == Device::DevicePosition::Open);
}