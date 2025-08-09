#pragma once

#include "ConfigParser.h"

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

private:
	void initLayout();

private:
	Cfg::DeviceConfigList _devices_cfg;

	// Widgets
	QPointer<QPushButton> _auto_mode_btn;
};

}