#include "AbstractCondition.h"
#include "DeviceStateManager.h"

#include "WeatherData.h"
#include "IndoorStation.h"

namespace Automation
{

namespace
{
template<typename T>
std::optional<double> getNumericFieldValue(const T& data, const QString& field)
{
	// Check WeatherData fields
	if constexpr (std::is_same_v<T, WeatherData>)
	{
		if (field == "outdoor_temp")
			return data.temperature;
		if (field == "wind_speed")
			return data.wind;
		if (field == "daylight")
			return data.daylight;
		if (field == "sun_south")
			return data.sun_south;
		if (field == "sun_east")
			return data.sun_east;
		if (field == "sun_west")
			return data.sun_west;
	}

	// Check IndoorData fields
	if constexpr (std::is_same_v<T, IndoorData>)
	{
		if (field == "indoor_temp")
			return data.temperature;
		if (field == "indoor_humidity")
			return data.humidity;
	}

	qWarning() << "Unknown field for data type " << typeid(T).name() << ": " << field;
	return {};
}

template<typename T>
std::optional<bool> getBooleanFieldValue(const T& data, const QString& field)
{
	// Check WeatherData fields
	if constexpr (std::is_same_v<T, WeatherData>)
	{
		if (field == "is_raining")
			return data.rain;
		if (field == "is_twighlight")
			return data.twighlight;
	}

	qWarning() << "Unknown boolean field for data type " << typeid(T).name() << ": " << field;
	return {};
}

bool evaluateNumericCondition(ConditionOperator op, std::optional<double> actual_value, double expected_value)
{
	if (!actual_value.has_value())
	{
		qWarning() << "evaluateNumericCondition called with empty optional";
		return false;
	}

	switch (op)
	{
	case ConditionOperator::GreaterThan:
		return actual_value > expected_value;
	case ConditionOperator::LessThan:
		return actual_value < expected_value;
	case ConditionOperator::EqualTo:
		return qFuzzyCompare(*actual_value, expected_value);
	case ConditionOperator::NotEqualTo:
		return !qFuzzyCompare(*actual_value, expected_value);
	case ConditionOperator::GreaterThanOrEqualTo:
		return actual_value >= expected_value;
	case ConditionOperator::LessThanOrEqualTo:
		return actual_value <= expected_value;

	default:
		qWarning() << "Unknown operator for evaluateNumericCondition: " << static_cast<int>(op);
		return false;
	}
}

bool evaluateBooleanCondition(bool actual_value, bool expected_value)
{
	return actual_value == expected_value;
}

template <typename T>
bool evaluateInHistoryDuration(const std::vector<T>& history, int duration_secs, std::function<bool(const T&)> evaluate_func)
{
	const QDateTime newest_timestamp = history.front().timestamp;

	// Check if the history covers the required duration
	if (!history.empty())
	{
		int history_duration_secs = history.back().timestamp.secsTo(newest_timestamp);
		if (history_duration_secs < duration_secs)
		{
			qDebug() << "History does not cover the required duration: " << history_duration_secs << " < " << duration_secs;
			return false;
		}
	}

	bool all_conditions_met = true;
	for (size_t i = 0; i < history.size(); ++i)
	{
		const T& data_point = history.at(i);
		int age_seconds = data_point.timestamp.secsTo(newest_timestamp);

		if (age_seconds > duration_secs)
			break; // This point is too old, and all subsequent ones must be old too

		if (!evaluate_func(data_point))
			return false;

	} // End of for loop of data points

	return all_conditions_met;
}

} // namespace

NumericThresholdCondition::NumericThresholdCondition(SensorDataSource source, const QString& field, ConditionOperator op, double value) :
	AbstractCondition(NumericThreshold), _source(source), _field(field), _op(op), _value(value)
{
}

bool NumericThresholdCondition::evaluate(const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history) const
{
	std::optional<double> value_opt = {};
	if (_source == SensorDataSource::WeatherData && !weather_history.empty())
		value_opt = getNumericFieldValue(weather_history.front(), _field);
	else if (_source == SensorDataSource::IndoorData && !indoor_history.empty())
		value_opt = getNumericFieldValue(indoor_history.front(), _field);

	return evaluateNumericCondition(_op, value_opt, _value);
}

BooleanStateCondition::BooleanStateCondition(SensorDataSource source, const QString& field, bool expected_value) :
	AbstractCondition(BooleanState), _source(source), _field(field), _expected_value(expected_value)
{
}

bool BooleanStateCondition::evaluate(const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history) const
{
	std::optional<bool> value_opt = {};
	if (_source == SensorDataSource::WeatherData && !weather_history.empty())
		value_opt = getBooleanFieldValue(weather_history.front(), _field);
	else if (_source == SensorDataSource::IndoorData && !indoor_history.empty())
		value_opt = getBooleanFieldValue(indoor_history.front(), _field);

	if (!value_opt.has_value())
	{
		qWarning() << "Unknown source or empty history for BooleanStateCondition: " << static_cast<int>(_source);
		return false;
	}

	return value_opt.value() == _expected_value;
}

NumericTimeDurationCondition::NumericTimeDurationCondition(SensorDataSource source, const QString& field, ConditionOperator op, double value, int duration_secs) :
	AbstractCondition(NumericTimeDuration), _source(source), _field(field), _op(op), _value(value), _duration_secs(duration_secs)
{
}

bool NumericTimeDurationCondition::evaluate(const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history) const
{
	if (_source == SensorDataSource::WeatherData && !weather_history.empty())
	{
		auto evaluate_func = [this](const WeatherData& weather_data_point) -> bool
			{
				// Evaluate with the current value for the data point
				auto value_opt = getNumericFieldValue(weather_data_point, _field);
				return evaluateNumericCondition(_op, value_opt, _value);
			};
		return evaluateInHistoryDuration<WeatherData>(weather_history, _duration_secs, evaluate_func);
	}
	else if (_source == SensorDataSource::IndoorData && !indoor_history.empty())
	{
		auto evaluate_func = [&](IndoorData weather_data_point)
			{
				auto value_opt = getNumericFieldValue(indoor_history.front(), _field);
				return evaluateNumericCondition(_op, value_opt, _value);
			};
		return evaluateInHistoryDuration<IndoorData>(indoor_history, _duration_secs, evaluate_func);
	}

	qWarning() << "NumericTimeDurationCondition - undefined source";
	return false;
}

BooleanTimeDurationCondition::BooleanTimeDurationCondition(SensorDataSource source, const QString& field, bool expected_value, int duration_secs) :
	AbstractCondition(BooleanTimeDuration), _source(source), _field(field), _expected_value(expected_value), _duration_secs(duration_secs)
{
}

bool BooleanTimeDurationCondition::evaluate(const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history) const
{
	if (_source == SensorDataSource::WeatherData && !weather_history.empty())
	{
		auto evaluate_func = [this](const WeatherData& weather_data_point) -> bool
			{
				// Evaluate with the current value for the data point
				auto value_opt = getBooleanFieldValue(weather_data_point, _field);
				return value_opt == _expected_value;
			};
		return evaluateInHistoryDuration<WeatherData>(weather_history, _duration_secs, evaluate_func);
	}
	else if (_source == SensorDataSource::IndoorData && !indoor_history.empty())
	{
		auto evaluate_func = [this](const IndoorData& weather_data_point) -> bool
			{
				// Evaluate with the current value for the data point
				auto value_opt = getBooleanFieldValue(weather_data_point, _field);
				return value_opt == _expected_value;
			};
		return evaluateInHistoryDuration<IndoorData>(indoor_history, _duration_secs, evaluate_func);
	}

	qWarning() << "BooleanTimeDurationCondition - undefined source";
	return false;
}

}