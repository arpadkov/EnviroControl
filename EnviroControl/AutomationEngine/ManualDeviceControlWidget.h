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
	void deviceUpPressed(QString device_id);
	void deviceDownPressed(QString device_id);
	void abortPressed();

private:
	void initLayout();

private:
	Cfg::DeviceConfigList _devices_cfg;
};

}