#include "AutomationWidget.h"
#include "DeviceStateWidget.h"
#include "ManualDeviceControlWidget.h"
#include "AutomationEngine.h"

#include <QtWidgets/QHBoxLayout>

#include <moc_AutomationWidget.cpp>

namespace Automation
{

AutomationWidget::AutomationWidget(const Cfg::DeviceConfigList& cfg, AutomationEngine* automation_engine, QWidget* parent)
	: QWidget(parent), _devices_cfg(cfg), _automation_engine(automation_engine)
{
	initLayout();

	// Connect ManualControlWidget
	connect(_manual_ctrl_w, &Automation::ManualDeviceControlWidget::deviceOpenPressed, _automation_engine, &Automation::AutomationEngine::onManualDeviceOpenRequest);
	connect(_manual_ctrl_w, &Automation::ManualDeviceControlWidget::deviceClosePressed, _automation_engine, &Automation::AutomationEngine::onManualDeviceCloseRequest);
	connect(_manual_ctrl_w, &Automation::ManualDeviceControlWidget::abortPressed, _automation_engine, &Automation::AutomationEngine::onAbort);

	connect(_manual_ctrl_w, &Automation::ManualDeviceControlWidget::automationModeChanged, this, &AutomationWidget::onAutomationModeChanged);
}

AutomationWidget::~AutomationWidget()
{
}

void AutomationWidget::onAutomationModeChanged(bool auto_mode)
{
	if (auto_mode)
		_automation_engine->setAutoMode();

	else
		_automation_engine->setManualMode();
}

void AutomationWidget::initLayout()
{
	auto main_layout = new QHBoxLayout();
	setLayout(main_layout);

	_manual_ctrl_w = new ManualDeviceControlWidget(_devices_cfg, this);
	main_layout->addWidget(_manual_ctrl_w);

	auto device_state_w = new DeviceStateWidget(_devices_cfg, this);
	main_layout->addWidget(device_state_w);
	connect(_automation_engine, &AutomationEngine::deviceStatesUpdated,
		device_state_w, &DeviceStateWidget::onDeviceStatesUpdated);
}

}