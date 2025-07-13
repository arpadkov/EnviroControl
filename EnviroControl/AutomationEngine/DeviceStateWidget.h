#pragma once

#include "ConfigParser.h"

#include <QWidget>
#include <QtCore/QPointer>

class QLabel;

namespace Device
{
class DeviceStates;
}

namespace Automation
{

/*
* Most likely just a temporary class to display the calculated states.
* The states come from AutomationEngine, even if Automation Engine is in manual mode.
*/
class DeviceStateWidget : public QWidget
{
	Q_OBJECT
public:
	DeviceStateWidget(const Cfg::DeviceConfigList& cfg, QWidget* parent = nullptr);
	~DeviceStateWidget() override;

public Q_SLOTS:
	void onDeviceStatesUpdated(const Device::DeviceStates& calulated_states);

private:
	void initLayout();

private:
	Cfg::DeviceConfigList _devices_cfg;
	std::map<QString, QPointer<QLabel>> _state_labels; // Maps device_id to QLabel for state display
};

}