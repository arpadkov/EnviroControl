#include "ManualDeviceControlWidget.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStyle>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>

#include <moc_ManualDeviceControlWidget.cpp>

namespace Automation
{

static const int ICON_SIZE = 64;
static const int BUTTON_SIZE = ICON_SIZE + 8;

namespace
{
QPushButton* createButton(QStyle::StandardPixmap icon_pm, QWidget* parent)
{
	auto button = new QPushButton(parent);
	auto icon = parent->style()->standardIcon(icon_pm);
	button->setIcon(icon);
	button->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
	button->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);
	return button;
}
}

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
	auto main_layout = new QHBoxLayout();
	setLayout(main_layout);

	int btn_count = 0;
	for (const auto& device_cfg : _devices_cfg.device_cfgs)
	{
		auto btns_layout = new QVBoxLayout();
		main_layout->addLayout(btns_layout);

		auto name_l = new QLabel(device_cfg.device_name);
		btns_layout->addWidget(name_l);

		auto up_btn = createButton(QStyle::SP_ArrowUp, this);
		btns_layout->addWidget(up_btn);
		connect(up_btn, &QPushButton::clicked, this, [this, device_cfg]()
			{
				Q_EMIT deviceUpPressed(device_cfg.device_id);
			});

		btns_layout->addStretch(1);

		auto down_btn = createButton(QStyle::SP_ArrowUp, this);
		btns_layout->addWidget(down_btn);
		connect(down_btn, &QPushButton::clicked, this, [this, device_cfg]()
			{
				Q_EMIT deviceDownPressed(device_cfg.device_id);
			});

		btn_count++;
		if (btn_count < _devices_cfg.device_cfgs.size())
			main_layout->addStretch(1);
	}

}


}