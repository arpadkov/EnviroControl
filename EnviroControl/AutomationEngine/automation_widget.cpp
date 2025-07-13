#include "AutomationWidget.h"
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
	connect(_manual_ctrl_w, &Automation::ManualDeviceControlWidget::deviceUpPressed, _automation_engine, &Automation::AutomationEngine::onManualDeviceUpRequest);
	connect(_manual_ctrl_w, &Automation::ManualDeviceControlWidget::deviceDownPressed, _automation_engine, &Automation::AutomationEngine::onManualDeviceDownRequest);
	connect(_manual_ctrl_w, &Automation::ManualDeviceControlWidget::abortPressed, _automation_engine, &Automation::AutomationEngine::onAbort);
}

AutomationWidget::~AutomationWidget()
{
}

void AutomationWidget::initLayout()
{
	auto main_layout = new QHBoxLayout();
	setLayout(main_layout);

	_manual_ctrl_w = new ManualDeviceControlWidget(_devices_cfg, this);
	main_layout->addWidget(_manual_ctrl_w);
}

}