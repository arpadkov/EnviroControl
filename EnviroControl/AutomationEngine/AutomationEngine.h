#pragma once

#include <QtCore/QObject>

namespace Automation
{
class AutomationEngine : public QObject
{
	Q_OBJECT
public:
	explicit AutomationEngine(QObject* parent = nullptr);
	~AutomationEngine();
};
}