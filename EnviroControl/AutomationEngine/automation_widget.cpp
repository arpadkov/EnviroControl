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

	// Connect ManualControlWidget to AutomationEngine
	connect(_manual_ctrl_w, &Automation::ManualDeviceControlWidget::deviceOpenPressed, _automation_engine, &Automation::AutomationEngine::onManualDeviceOpenRequest);
	connect(_manual_ctrl_w, &Automation::ManualDeviceControlWidget::deviceClosePressed, _automation_engine, &Automation::AutomationEngine::onManualDeviceCloseRequest);
	connect(_manual_ctrl_w, &Automation::ManualDeviceControlWidget::abortPressed, _automation_engine, &Automation::AutomationEngine::onAbort);
	connect(_manual_ctrl_w, &Automation::ManualDeviceControlWidget::automationModeChangeRequest, _automation_engine, &AutomationEngine::onAutomationModeChangeRequest);

	connect(_automation_engine, &AutomationEngine::deviceMovementStarted, _manual_ctrl_w, &ManualDeviceControlWidget::onDeviceMovementStarted);
	connect(_automation_engine, &AutomationEngine::deviceMovementFinished, _manual_ctrl_w, &ManualDeviceControlWidget::onDeviceMovementFinished);
	connect(_automation_engine, &AutomationEngine::deviceStatesUpdated, _manual_ctrl_w, &ManualDeviceControlWidget::onDeviceStatesUpdated);

	// Connect AutomationEngine response signals to this
	connect(_automation_engine, &Automation::AutomationEngine::automationModeChanged, _manual_ctrl_w, &Automation::ManualDeviceControlWidget::onAutomationModeChanged);
}

AutomationWidget::~AutomationWidget()
{
}

bool AutomationWidget::isInAutoMode() const
{
	return _automation_engine->isInAutoMode();
}

void AutomationWidget::initLayout()
{
	auto main_layout = new QHBoxLayout();
	setLayout(main_layout);

	_manual_ctrl_w = new ManualDeviceControlWidget(_devices_cfg, this);
	main_layout->addWidget(_manual_ctrl_w);

	main_layout->addStretch();

	auto device_state_w = new DeviceStateWidget(_devices_cfg, this);
	main_layout->addWidget(device_state_w);
	connect(_automation_engine, &AutomationEngine::deviceStatesUpdated,
		device_state_w, &DeviceStateWidget::onDeviceStatesUpdated);
}

}