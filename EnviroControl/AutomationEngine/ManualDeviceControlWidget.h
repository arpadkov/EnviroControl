#pragma once

#include "ConfigParser.h"

#include <QWidget>

namespace Automation
{

class ManualDeviceControlWidget : public QWidget
{
	Q_OBJECT
public:
	ManualDeviceControlWidget(const Cfg::DeviceConfigList& cfg, QWidget* parent = nullptr);
	~ManualDeviceControlWidget() override;

Q_SIGNALS:
	void deviceOpenPressed(QString device_id);
	void deviceClosePressed(QString device_id);
	void abortPressed();
	void automationModeChanged(bool auto_mode);

private:
	void initLayout();

private:
	Cfg::DeviceConfigList _devices_cfg;
};

}