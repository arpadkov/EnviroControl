#include "ManualDeviceControlWidget.h"
#include "DeviceState.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>

#include <moc_ManualDeviceControlWidget.cpp>

namespace Automation
{

static const int ICON_SIZE = 64;
static const int BUTTON_SIZE = ICON_SIZE + 8;

namespace
{
QPushButton* createButton(const QString& icon_name, QWidget* parent)
{
	auto button = new QPushButton(parent);
	auto icon = QIcon(QString(":/manual_ctrl/icons/%1.svg").arg(icon_name));
	button->setIcon(icon);
	button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
	button->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
	return button;
}

bool leaveAutomationModeDialog(const Device::DeviceStates& calculated_states, const Cfg::DeviceConfig& device_cfg)
{
	QMessageBox msg_box;
	msg_box.setText(QString("The device \"%1\" has calculated position \"%2\"").
		arg(device_cfg.device_name, calculated_states.deviceStateAsString(device_cfg.device_id)));
	msg_box.setInformativeText("Do you want to set manual mode?");
	msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msg_box.setDefaultButton(QMessageBox::No);

	return (msg_box.exec() == QMessageBox::Yes);
}

/*
* Only emit if the device has no calculated state (is free to do whatever), or if the user confirms leaving automation mode
*/
bool emitManualRequestSignal(const Device::DeviceStates& calculated_states, const Cfg::DeviceConfig& device_cfg)
{
	bool is_state_unknown = calculated_states.getDevicePosition(device_cfg.device_id) == Device::DevicePosition::Unknown;
	return is_state_unknown || leaveAutomationModeDialog(calculated_states, device_cfg);
}

}	// namespace

ManualDeviceControlWidget::ManualDeviceControlWidget(const Cfg::DeviceConfigList& cfg, QWidget* parent)
	: QWidget(parent), _devices_cfg(cfg)
{
	initLayout();
}

ManualDeviceControlWidget::~ManualDeviceControlWidget()
{
}

void ManualDeviceControlWidget::onAutomationModeChanged(bool auto_mode)
{
	_auto_mode_btn->setChecked(auto_mode);
}

void ManualDeviceControlWidget::onDeviceMovementFinished(const Device::DeviceState& state)
{
	auto it = _device_buttons.find(state.device_id);
	if (it == _device_buttons.end())
	{
		qWarning("ManualDeviceControlWidget::onDeviceMovementFinished: Unknown device_id '%s'", state.device_id.toStdString().c_str());
		return;
	}

	// Update buttons
	auto btn_pair = it->second;
	switch (state.position)
	{
	case Device::DevicePosition::Open:
		btn_pair.first->setChecked(true);
		btn_pair.second->setChecked(false);
		break;
	case Device::DevicePosition::Closed:
		btn_pair.first->setChecked(false);
		btn_pair.second->setChecked(true);
		break;
	}
}

void ManualDeviceControlWidget::onDeviceStatesUpdated(const Device::DeviceStates& states)
{
	_calculated_device_states = states;
}

inline void ManualDeviceControlWidget::initLayout()
{
	auto main_layout = new QHBoxLayout();
	setLayout(main_layout);

	for (const auto& device_cfg : _devices_cfg.device_cfgs)
	{
		auto btns_layout = new QVBoxLayout();
		main_layout->addLayout(btns_layout);

		auto name_l = new QLabel(device_cfg.device_name);
		btns_layout->addWidget(name_l);

		// Open button
		auto open_btn = createButton(device_cfg.open_icon, this);
		open_btn->setCheckable(true);
		btns_layout->addWidget(open_btn);

		btns_layout->addStretch(1);

		// Close button
		auto close_btn = createButton(device_cfg.close_icon, this);
		close_btn->setCheckable(true);
		btns_layout->addWidget(close_btn);

		auto uncheck_btns = [open_btn, close_btn]()
			{
				open_btn->setChecked(false);
				close_btn->setChecked(false);
			};

		// Define connections: need to know about both buttons
		connect(open_btn, &QPushButton::clicked, this, [this, device_cfg, uncheck_btns, open_btn]()
			{
				if (!emitManualRequestSignal(_calculated_device_states, device_cfg))
				{
					open_btn->setCheckable(false); // Uncheck the button, if the user cancelled
					return;
				}

				uncheck_btns();
				Q_EMIT deviceOpenPressed(device_cfg.device_id);
			});
		connect(close_btn, &QPushButton::clicked, this, [this, device_cfg, uncheck_btns, close_btn]()
			{
				if (!emitManualRequestSignal(_calculated_device_states, device_cfg))
				{
					close_btn->setCheckable(false); // Uncheck the button, if the user cancelled
					return;
				}

				uncheck_btns();
				Q_EMIT deviceClosePressed(device_cfg.device_id);
			});

		// Save buttons pointers
		_device_buttons[device_cfg.device_id] = std::make_pair(open_btn, close_btn);

		main_layout->addStretch(1);
	}

	// Reset and auto buttons
	auto ctrl_layout = new QVBoxLayout();
	main_layout->addLayout(ctrl_layout);

	auto reset_btn = createButton("abort_ops", this);
	ctrl_layout->addWidget(reset_btn);
	connect(reset_btn, &QPushButton::clicked, this, &ManualDeviceControlWidget::abortPressed);

	_auto_mode_btn = createButton("auto_mode", this);
	ctrl_layout->addWidget(_auto_mode_btn);
	_auto_mode_btn->setCheckable(true);
	_auto_mode_btn->setChecked(false);
	connect(_auto_mode_btn, &QPushButton::clicked, this, [this]()
		{
			// Just emit the signal, the buttons state is changed, once AutomationEngine is set to auto mode
			Q_EMIT automationModeChangeRequest(_auto_mode_btn->isChecked());
		});
}

}