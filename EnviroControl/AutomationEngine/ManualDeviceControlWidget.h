#pragma once

#include "ConfigParser.h"
#include "DeviceState.h"

#include <QtWidgets/QWidget>
#include <QtCore/QPointer>

class QPushButton;

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
	void automationModeChangeRequest(bool auto_mode);

public Q_SLOTS:
	void onAutomationModeChanged(bool auto_mode);
	void onDeviceMovementFinished(const Device::DeviceState& state);
	void onDeviceStatesUpdated(const Device::DeviceStates& states);

private:
	void initLayout();

private:
	Cfg::DeviceConfigList _devices_cfg;

	Device::DeviceStates _calculated_device_states;

	// Widgets
	QPointer<QPushButton> _auto_mode_btn;
	std::map<QString, std::pair<QPushButton*, QPushButton*>> _device_buttons; // device_id -> <open_btn, close_btn>
};

}