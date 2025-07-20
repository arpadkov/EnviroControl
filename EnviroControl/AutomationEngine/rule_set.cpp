#include "RuleSet.h"

#include <QtCore/QJsonObject>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QFile>


namespace Automation
{

bool RuleSet::loadFromJson(const QString& file_path)
{
	_rules.clear();


	QFile config_file(file_path);

	if (!config_file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qCritical() << "RuleSet: Could not open config file:" << config_file.fileName() << "Reason:" << config_file.errorString();
		return false;
	}

	QByteArray json_data = config_file.readAll();
	config_file.close();

	QJsonParseError parse_error;
	QJsonDocument doc = QJsonDocument::fromJson(json_data, &parse_error);

	if (doc.isNull())
	{
		qCritical() << "RuleSet: Failed to parse JSON from config file:" << file_path << "Reason:" << parse_error.errorString();
		return false;
	}

	if (!doc.isObject())
	{
		qCritical() << "RuleSet: JSON root is not an object in config file:" << file_path;
		return false;
	}

	QJsonObject json = doc.object();

	if (!json.contains("rules") || !json["rules"].isArray())
	{
		qWarning() << "RuleSet: JSON does not contain rules";
		return false;
	}

	const auto& rules_array = json["rules"].toArray();
	for (const auto& rule_value : rules_array)
	{

		if (!rule_value.isObject())
		{
			qWarning() << "RuleSet: Rule entry is not an object. Skipping.";
			continue;
		}
		QJsonObject rule_json = rule_value.toObject();

		Rule rule;
		if (rule_json.contains("id") || rule_json["id"].isString())
			rule.id = rule_json["id"].toString();
		else
		{

			qWarning() << "Rule missing 'id'. Skipping.";
			continue;
		}

		if (rule_json.contains("device_id") && rule_json["device_id"].isString())
			rule.device_id = rule_json["device_id"].toString();
		else
		{
			qWarning() << "Rule '" << rule.id << "' missing 'device_id'. Skipping.";
			continue;
		}

		if (rule_json.contains("priority") && rule_json["priority"].isDouble())
			rule.priority = static_cast<int>(rule_json["priority"].toDouble());
		else
		{
			qWarning() << "Rule '" << rule.id << "' missing 'priority'. Skipping.";
			continue;
		}

		if (rule_json.contains("action") && rule_json["action"].isString())
		{
			QString actionStr = rule_json["action"].toString().toLower();
			if (actionStr == "open")
				rule.position = Device::DevicePosition::Open;
			else if (actionStr == "close")
				rule.position = Device::DevicePosition::Closed;
			else
			{
				qWarning() << "Rule '" << rule.id << "' has unknown action:" << actionStr << ". Skipping.";
				continue;
			}
		}
		else
		{
			qWarning() << "Rule '" << rule.id << "' missing 'action'. Skipping.";
			continue;
		}

		if (rule_json.contains("conditions") && rule_json["conditions"].isArray())
		{
			QJsonArray conditions_array = rule_json["conditions"].toArray();
			for (const QJsonValue& condition_value : conditions_array)
			{
				if (!condition_value.isObject())
				{
					qWarning() << "Rule '" << rule.id << "': Condition entry is not an object. Skipping.";
					continue;
				}
				std::unique_ptr<AbstractCondition> condition = parseCondition(condition_value.toObject());
				if (condition)
				{
					rule.conditions.push_back(std::move(condition));
				}
				else
				{
					qWarning() << "Rule '" << rule.id << "': Failed to parse condition. Skipping.";
				}
			}
		}
		else
		{
			qWarning() << "Rule '" << rule.id << "' missing 'conditions' array. Skipping."; continue;
		}

		_rules.push_back(std::move(rule));

	}

	return true;
}

const std::vector<Rule>& RuleSet::getRules() const
{
	return _rules;
}

void RuleSet::setRules(std::vector<Rule>&& rules)
{
	_rules = std::move(rules);
}

std::unique_ptr<AbstractCondition> RuleSet::parseCondition(const QJsonObject& json) const
{
	if (!json.contains("type") || !json["type"].isString())
	{
		qWarning() << "Condition missing 'type'.";
		return nullptr;
	}
	QString type_str = json["type"].toString().toLower();

	if (!json.contains("sensor_type") || !json["sensor_type"].isString())
	{
		qWarning() << "Condition missing 'sensor_type'.";
		return nullptr;
	}
	QString sensor_type_str = json["sensor_type"].toString().toLower();
	SensorDataSource sensor_source = SensorDataSource::Unknown;
	if (sensor_type_str == "weather_data")
		sensor_source = SensorDataSource::WeatherData;
	else if (sensor_type_str == "indoor_data")
		sensor_source = SensorDataSource::IndoorData;
	else
	{
		qWarning() << "Unknown sensor_type:" << sensor_type_str;
		return nullptr;
	}

	QString field;
	if (json.contains("field") && json["field"].isString())
	{
		field = json["field"].toString();
	}
	else
	{
		qWarning() << "Condition missing 'field'.";
		return nullptr;
	}

	// Parse common operator and value for numeric conditions
	ConditionOperator op = ConditionOperator::Unknown;
	double value = 0.0;
	if (type_str == "numeric_threshold" || type_str == "numeric_time_duration")
	{
		if (json.contains("operator") && json["operator"].isString())
		{
			op = stringToConditionOperator(json["operator"].toString());
			if (op == ConditionOperator::Unknown)
			{
				qWarning() << "Unknown operator type.";
				return nullptr;
			}
		}
		else
		{
			qWarning() << "Numeric condition missing 'operator'."; return nullptr;
		}

		if (json.contains("value") && json["value"].isDouble())
		{
			value = json["value"].toDouble();
		}
		else
		{
			qWarning() << "Numeric condition missing 'value'."; return nullptr;
		}
	}


	if (type_str == "numeric_threshold")
	{
		return std::make_unique<NumericThresholdCondition>(sensor_source, field, op, value);
	}
	else if (type_str == "boolean_state")
	{
		if (json.contains("expected_value") && json["expected_value"].isBool())
		{
			bool expectedValue = json["expected_value"].toBool();
			return std::make_unique<BooleanStateCondition>(sensor_source, field, expectedValue);
		}
		else
		{
			qWarning() << "Boolean condition missing 'expected_value'.";
			return nullptr;
		}
	}
	else if (type_str == "numeric_time_duration")
	{
		if (json.contains("duration_seconds") && json["duration_seconds"].isDouble())
		{
			int durationSeconds = static_cast<int>(json["duration_seconds"].toDouble());
			return std::make_unique<NumericTimeDurationCondition>(sensor_source, field, op, value, durationSeconds);
		}
		else
		{
			qWarning() << "Numeric time duration condition missing 'duration_seconds'.";
			return nullptr;
		}
	}
	else
	{
		qWarning() << "Unknown condition type:" << type_str;
		return nullptr;
	}
}


}