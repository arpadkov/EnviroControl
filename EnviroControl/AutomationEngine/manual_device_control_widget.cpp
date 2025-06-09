#include "ManualDeviceControlWidget.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStyle>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>

#include <moc_ManualDeviceControlWidget.cpp>

namespace Automation
{

ManualDeviceControlWidget::ManualDeviceControlWidget(const Cfg::DeviceConfigList& cfg, QWidget* parent)
	: QWidget(parent), _devices_cfg(cfg)
{
	initLayout();
}

ManualDeviceControlWidget::~ManualDeviceControlWidget()
{
}

inline void ManualDeviceControlWidget::initLayout()
{
	auto layout = new QHBoxLayout();
	setLayout(layout);

	for (const auto& device_cfg : _devices_cfg.device_cfgs)
	{
		auto btns_layout = new QVBoxLayout();
		layout->addItem(btns_layout);

		auto name_l = new QLabel(device_cfg.device_name);
		btns_layout->addWidget(name_l);

		auto up_btn = new QPushButton(this);
		auto up_icon = style()->standardIcon(QStyle::SP_ArrowUp);
		up_btn->setIcon(up_icon);
		btns_layout->addWidget(up_btn);
		connect(up_btn, &QPushButton::clicked, this, [this, device_cfg]()
			{
				Q_EMIT deviceUpPressed(device_cfg.device_id);
			});

		auto down_btn = new QPushButton(this);
		auto down_icon = style()->standardIcon(QStyle::SP_ArrowDown);
		down_btn->setIcon(down_icon);
		btns_layout->addWidget(down_btn);
		connect(down_btn, &QPushButton::clicked, this, [this, device_cfg]()
			{
				Q_EMIT deviceDownPressed(device_cfg.device_id);
			});
	}

}


}