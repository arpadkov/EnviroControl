#include "DeviceStateWidget.h"

#include "DeviceStateManager.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>


namespace Automation
{

DeviceStateWidget::DeviceStateWidget(const Cfg::DeviceConfigList& cfg, QWidget* parent)
	: QWidget(parent), _devices_cfg(cfg)
{
	initLayout();
}

DeviceStateWidget::~DeviceStateWidget()
{
}

void DeviceStateWidget::initLayout()
{
	auto main_layout = new QHBoxLayout();
	setLayout(main_layout);

	for (const auto& device_cfg : _devices_cfg.device_cfgs)
	{
		auto state_layout = new QVBoxLayout();
		main_layout->addLayout(state_layout);

		auto name_l = new QLabel(device_cfg.device_name);
		state_layout->addWidget(name_l);

		auto state_l = new QLabel(this);
		state_layout->addWidget(state_l);

		_state_labels[device_cfg.device_id] = state_l;
	}
}

void DeviceStateWidget::onDeviceStatesUpdated(const Device::DeviceStates& calulated_states)
{
	for (const auto& state : calulated_states.states)
	{
		auto it = _state_labels.find(state.device_id);
		if (it != _state_labels.end())
		{
			auto state_label = it->second;
			if (!state_label)
			{
				qWarning() << "DeviceStateWidget::onDeviceStatesUpdated: QLabel for device ID" << state.device_id << "is null.";
				continue;
			}

			const auto& device_cfg = std::find_if(_devices_cfg.device_cfgs.begin(), _devices_cfg.device_cfgs.end(),
				[&state](const Cfg::DeviceConfig& cfg)
				{
					return cfg.device_id == state.device_id;
				});

			if (device_cfg == _devices_cfg.device_cfgs.end())
			{
				qWarning() << "DeviceStateWidget::onDeviceStatesUpdated: Device ID" << state.device_id << "not found in device configurations.";
				continue;
			}

			// Update the label icon
			QString icon_name;
			if (state.position == Device::DevicePosition::Open)
				icon_name = device_cfg->open_icon;
			else if (state.position == Device::DevicePosition::Closed)
				icon_name = device_cfg->close_icon;
			else
				icon_name = "unknown";


			auto icon = QIcon(QString(":/manual_ctrl/icons/%1.svg").arg(icon_name));
			state_label->setPixmap(icon.pixmap(QSize(32, 32)));
			// Update the label text
		}
		else
		{
			qWarning() << "DeviceStateWidget::onDeviceStatesUpdated: Device ID" << state.device_id << "not found in state labels.";
		}
	}
}

}