#pragma once

#include "ConfigParser.h"

#include <QWidget>
#include <QtCore/QPointer>

namespace Automation
{
class AutomationEngine;
class ManualDeviceControlWidget;
}

namespace Automation
{

class AutomationWidget : public QWidget
{
	Q_OBJECT
public:
	AutomationWidget(const Cfg::DeviceConfigList& cfg, AutomationEngine* automation_engine, QWidget* parent = nullptr);
	~AutomationWidget() override;

Q_SIGNALS:
	void deviceOpenPressed(QString device_id);
	void deviceClosePressed(QString device_id);
	void abortPressed();

private:
	void initLayout();

private:
	Cfg::DeviceConfigList _devices_cfg;
	QPointer<AutomationEngine> _automation_engine;
	QPointer<ManualDeviceControlWidget> _manual_ctrl_w;
};

}