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

template <typename T>
bool evaluateInHistoryDuration(const std::vector<T>& history, const QString& field, ConditionOperator op, double value, int duration_secs, SensorDataSource source)
{
	if (history.empty())
	{
		qWarning() << "Empty history for evaluateInHistoryDuration: " << static_cast<int>(source);
		return false;
	}

	const QDateTime newest_timestamp = history.front().timestamp;

	// Check if the history covers the required duration
	if (!history.empty())
	{
		int history_duration_secs = history.back().timestamp.secsTo(newest_timestamp);
		if (history_duration_secs < duration_secs)
		{
			qWarning() << "History does not cover the required duration: " << history_duration_secs << " < " << duration_secs;
			return false;
		}
	}

	bool all_conditions_met = true;
	for (size_t i = 0; i < history.size(); ++i)
	{
		const T& data_pont = history.at(i);
		int age_seconds = data_pont.timestamp.secsTo(newest_timestamp);

		if (age_seconds > duration_secs)
			break; // This point is too old, and all subsequent ones must be old too

		std::optional<double> value_opt = getNumericFieldValue(data_pont, field);
		if (!value_opt.has_value())
		{
			qWarning() << "Unknown field for data point in evaluateInHistoryDuration: " << field;
			return false;
		}

		double actual_value = value_opt.value();
		bool condition_met = false; // For current data point
		switch (op)
		{
		case ConditionOperator::GreaterThan:
			condition_met = actual_value > value;
			break;
		case ConditionOperator::LessThan:
			condition_met = actual_value < value;
			break;
		case ConditionOperator::EqualTo:
			condition_met = qFuzzyCompare(actual_value, value);
			break;
		case ConditionOperator::NotEqualTo:
			condition_met = !qFuzzyCompare(actual_value, value);
			break;
		case ConditionOperator::GreaterThanOrEqualTo:
			condition_met = actual_value >= value;
			break;
		case ConditionOperator::LessThanOrEqualTo:
			condition_met = actual_value <= value;
			break;
		default:
			qWarning() << "Unknown operator for NumericThresholdCondition: " << static_cast<int>(op);
			return false;
		}

		if (!condition_met)
		{
			all_conditions_met = false;
			break; // No need to check further, one condition failed
		}
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

	if (!value_opt.has_value())
	{
		qWarning() << "Unknown source or empty history for NumericThresholdCondition: " << static_cast<int>(_source);
		return false;
	}

	double actual_value = value_opt.value();
	switch (_op)
	{
	case ConditionOperator::GreaterThan:
		return actual_value > _value;
	case ConditionOperator::LessThan:
		return actual_value < _value;
	case ConditionOperator::EqualTo:
		return qFuzzyCompare(actual_value, _value);
	case ConditionOperator::NotEqualTo:
		return !qFuzzyCompare(actual_value, _value);
	case ConditionOperator::GreaterThanOrEqualTo:
		return actual_value >= _value;
	case ConditionOperator::LessThanOrEqualTo:
		return actual_value <= _value;

	default:
		qWarning() << "Unknown operator for NumericThresholdCondition: " << static_cast<int>(_op);
		return false;
	}
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
		return evaluateInHistoryDuration(weather_history, _field, _op, _value, _duration_secs, _source);
	else if (_source == SensorDataSource::IndoorData && !indoor_history.empty())
		return evaluateInHistoryDuration(indoor_history, _field, _op, _value, _duration_secs, _source);

	else
	{
		qWarning() << "Unknown source or empty history for NumericTimeDurationCondition: " << static_cast<int>(_source);
		return false;
	}
}



}