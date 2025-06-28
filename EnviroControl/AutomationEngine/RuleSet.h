#pragma once

#include "AbstractCondition.h"
#include "DeviceState.h"

class QJsonObject;

namespace Automation
{
struct Rule
{
	QString id;
	QString device_id;
	int priority = 0;
	Device::DevicePosition position = Device::DevicePosition::Unknown;
	std::vector<std::unique_ptr<AbstractCondition>> conditions;
};

class RuleSet
{
public:
	RuleSet() = default;
	bool loadFromJson(const QString& file_path);
	const std::vector<Rule>& getRules() const;

private:
	std::unique_ptr<AbstractCondition> parseCondition(const QJsonObject& json) const;

	std::vector<Rule> _rules;

};



}