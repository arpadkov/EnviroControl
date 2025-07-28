#pragma once

#include <QtCore/QString>

struct WeatherData;
struct IndoorData;

namespace Automation
{

enum class SensorDataSource
{
	Unknown,
	WeatherData,
	IndoorData,
};

enum class ConditionOperator
{
	Unknown,
	GreaterThan,
	LessThan,
	EqualTo,
	NotEqualTo,
	GreaterThanOrEqualTo,
	LessThanOrEqualTo
};

inline ConditionOperator stringToConditionOperator(const QString& str)
{
	if (str == "gt") return ConditionOperator::GreaterThan;
	if (str == "lt") return ConditionOperator::LessThan;
	if (str == "eq") return ConditionOperator::EqualTo;
	if (str == "ne") return ConditionOperator::NotEqualTo;
	if (str == "gte") return ConditionOperator::GreaterThanOrEqualTo;
	if (str == "lte") return ConditionOperator::LessThanOrEqualTo;
	return ConditionOperator::Unknown;
}

inline QString conditionOperatorToString(ConditionOperator op)
{
	switch (op)
	{
	case ConditionOperator::GreaterThan: return "gt";
	case ConditionOperator::LessThan: return "lt";
	case ConditionOperator::EqualTo: return "eq";
	case ConditionOperator::NotEqualTo: return "ne";
	case ConditionOperator::GreaterThanOrEqualTo: return "gte";
	case ConditionOperator::LessThanOrEqualTo: return "lte";
	default: return "unknown";
	}
}

class AbstractCondition
{
public:
	enum Type
	{
		Unknown,
		NumericThreshold,
		BooleanState,
		NumericTimeDuration,
		BooleanTimeDuration
	};

	AbstractCondition(Type type) : _type(type)
	{
	};
	virtual ~AbstractCondition() = default;

	Type type() const
	{
		return _type;
	}

	virtual bool evaluate(const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history) const = 0;

protected:
	Type _type;
};

class NumericThresholdCondition : public AbstractCondition
{
public:
	NumericThresholdCondition(SensorDataSource source, const QString& field, ConditionOperator op, double value);

	bool evaluate(const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history) const override;

private:
	SensorDataSource _source;
	QString _field;
	ConditionOperator _op;
	double _value;
};

class BooleanStateCondition : public AbstractCondition
{
public:
	BooleanStateCondition(SensorDataSource source, const QString& field, bool expected_value);

	bool evaluate(const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history) const override;

private:
	SensorDataSource _source;
	QString _field;
	bool _expected_value;
};

class NumericTimeDurationCondition : public AbstractCondition
{
public:
	NumericTimeDurationCondition(SensorDataSource source, const QString& field, ConditionOperator op, double value, int duration_secs);

	bool evaluate(const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history) const override;

private:
	SensorDataSource _source;
	QString _field;
	ConditionOperator _op;
	double _value;
	int _duration_secs;
};

class BooleanTimeDurationCondition : public AbstractCondition
{
public:
	BooleanTimeDurationCondition(SensorDataSource source, const QString& field, bool expected_value, int duration_secs);

	bool evaluate(const std::vector<WeatherData>& weather_history, const std::vector<IndoorData>& indoor_history) const override;

private:
	SensorDataSource _source;
	QString _field;
	bool _expected_value;
	int _duration_secs;
};

}