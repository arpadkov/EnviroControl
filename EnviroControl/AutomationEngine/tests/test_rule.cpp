#include "gtest/gtest.h"

#include "WeatherDataCreator.h"

#include "RuleSet.h"
#include "RulesProcessor.h"
#include "WeatherStation.h"
#include "IndoorStation.h"
#include "DeviceState.h"

using namespace Automation;

Rule createRuleWithoutCondition()
{
	Rule rule;
	rule.id = "id";
	rule.device_id = "device_id";
	rule.priority = 10;
	rule.position = Device::DevicePosition::Open;
	return rule;
}

TEST(RuleTest, OnePassOneFail)
{
	auto rain_pass_condition = std::make_unique<BooleanStateCondition>(SensorDataSource::WeatherData, "is_raining", true);
	auto high_wind_pass_condition = std::make_unique<NumericThresholdCondition>(SensorDataSource::WeatherData, "wind_speed", ConditionOperator::GreaterThan, 25);

	// Make rainy, but low wind WeatherData
	std::vector<WeatherData> weather_history;
	auto rainy_weather = WeatherDataCreator::createRainy(QDateTime::currentDateTime());
	weather_history.push_back(rainy_weather);
	std::vector<IndoorData> indoor_history;

	auto rule = createRuleWithoutCondition();
	rule.conditions.push_back(std::move(rain_pass_condition));
	rule.conditions.push_back(std::move(high_wind_pass_condition));

	auto rule_passed = RulesProcessor::evaluateRule(rule, weather_history, indoor_history);
	EXPECT_FALSE(rule_passed);
}

TEST(RuleTest, ThreePass)
{
	auto rain_pass_condition = std::make_unique<BooleanStateCondition>(SensorDataSource::WeatherData, "is_raining", true);
	auto low_wind_pass_condition = std::make_unique<NumericThresholdCondition>(SensorDataSource::WeatherData, "wind_speed", ConditionOperator::LessThan, 25);
	auto no_sun_pass_condition = std::make_unique<NumericThresholdCondition>(SensorDataSource::WeatherData, "daylight", ConditionOperator::LessThan, 120);

	// Make rainy, but low wind, low sun WeatherData
	std::vector<WeatherData> weather_history;
	auto rainy_weather = WeatherDataCreator::createRainy(QDateTime::currentDateTime());
	weather_history.push_back(rainy_weather);
	std::vector<IndoorData> indoor_history;

	auto rule = createRuleWithoutCondition();
	rule.conditions.push_back(std::move(rain_pass_condition));
	rule.conditions.push_back(std::move(low_wind_pass_condition));
	rule.conditions.push_back(std::move(no_sun_pass_condition));

	auto rule_passed = RulesProcessor::evaluateRule(rule, weather_history, indoor_history);
	EXPECT_TRUE(rule_passed);
}