#include "gtest/gtest.h"

#include "WeatherDataCreator.h"

#include "RuleSet.h"
#include "WeatherData.h"
#include "IndoorStation.h"

using namespace Automation;

TEST(ConditionTest, BooleanStateCondition)
{
	std::vector<WeatherData> weather_history;
	std::vector<IndoorData> indoor_history;

	weather_history.push_back(WeatherDataCreator::createRainy(QDateTime::currentDateTime()));

	auto pass_rain = BooleanStateCondition(SensorDataSource::WeatherData, "is_raining", true);
	auto pass_no_rain = BooleanStateCondition(SensorDataSource::WeatherData, "is_raining", false);

	EXPECT_TRUE(pass_rain.evaluate(weather_history, indoor_history));
	EXPECT_FALSE(pass_no_rain.evaluate(weather_history, indoor_history));
}

TEST(ConditionTest, NumericThresholdCondition)
{
	std::vector<WeatherData> weather_history;
	std::vector<IndoorData> indoor_history;

	weather_history.push_back(WeatherDataCreator::createWindy(QDateTime::currentDateTime(), 25));

	auto pass_above_24 = NumericThresholdCondition(SensorDataSource::WeatherData, "wind_speed", ConditionOperator::GreaterThan, 24);
	auto pass_above_30 = NumericThresholdCondition(SensorDataSource::WeatherData, "wind_speed", ConditionOperator::GreaterThan, 30);

	EXPECT_TRUE(pass_above_24.evaluate(weather_history, indoor_history));
	EXPECT_FALSE(pass_above_30.evaluate(weather_history, indoor_history));
}

TEST(ConditionTest, NumericThresholdConditionIndoor)
{
	std::vector<WeatherData> weather_history;
	std::vector<IndoorData> indoor_history;

	indoor_history.push_back(WeatherDataCreator::createIndoorData(QDateTime::currentDateTime(), 28));

	auto pass_indoor_hot = NumericThresholdCondition(SensorDataSource::IndoorData, "indoor_temp", ConditionOperator::GreaterThan, 25);
	auto pass_indoor_cold = NumericThresholdCondition(SensorDataSource::IndoorData, "indoor_temp", ConditionOperator::LessThan, 10);

	EXPECT_TRUE(pass_indoor_hot.evaluate(weather_history, indoor_history));
	EXPECT_FALSE(pass_indoor_cold.evaluate(weather_history, indoor_history));
}

TEST(ConditionTest, NumericTimeDurationCondition)
{
	// Weather data for 5 minutes wind (last 4 minutes low)
	// one condition requires more than 10 minutes no wind -> fail
	// one condition requires more than 3 minutes no wind -> pass
	std::vector<WeatherData> weather_history;
	std::vector<IndoorData> indoor_history;

	// First one is newest
	auto now = QDateTime::currentDateTime();
	weather_history.push_back(WeatherDataCreator::createWindy(now, 24));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 1) , 23));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 2) , 24.5));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 3) , 24.8));
	weather_history.push_back(WeatherDataCreator::createWindy(now.addSecs(-60 * 4) , 26));

	auto pass_low_wind_10_min = NumericTimeDurationCondition(SensorDataSource::WeatherData, "wind_speed", ConditionOperator::LessThan, 25, 60 * 10);
	auto pass_low_wind_3_min = NumericTimeDurationCondition(SensorDataSource::WeatherData, "wind_speed", ConditionOperator::LessThan, 25, 60 * 3);

	EXPECT_FALSE(pass_low_wind_10_min.evaluate(weather_history, indoor_history));
	EXPECT_TRUE(pass_low_wind_3_min.evaluate(weather_history, indoor_history));
}