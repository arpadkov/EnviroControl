#include "ErrorDetailsWidget.h"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFrame>
#include <QtWidgets/QListWidget>

ErrorDetailsWidget::ErrorDetailsWidget(QWidget* parent) : QWidget(parent)
{
	initLayout();
	setVisible(false);
}

ErrorDetailsWidget::~ErrorDetailsWidget()
{
}

void ErrorDetailsWidget::onErrorOccurred(const QString& error)
{
	addError(error);
}

void ErrorDetailsWidget::addError(const QString& error)
{
	_error_list_w->addItem(error);

	if (!isVisible())
		setVisible(true);
}

void ErrorDetailsWidget::initLayout()
{
	auto layout = new QHBoxLayout(this);
	setLayout(layout);

	auto frame = new QFrame(this);
	frame->setFrameShape(QFrame::Box);
	frame->setFrameShadow(QFrame::Sunken);
	frame->setStyleSheet("QFrame { background-color: #fce4e4; border: 1px solid #d32f2f; border-radius: 4px; }");
	layout->addWidget(frame);

	auto frame_layout = new QVBoxLayout(frame);
	frame_layout->setContentsMargins(0, 0, 0, 0);
	frame_layout->setSpacing(0);

	_error_list_w = new QListWidget(frame);
	_error_list_w->setStyleSheet("QListWidget { background: #fce4e4; border-top: 1px solid #d32f2f; }");
	_error_list_w->setHidden(true);

	auto expand_btn = new QPushButton("Expand", frame);
	expand_btn->setStyleSheet("QPushButton { text-align: left; padding: 5px; border: none; background: transparent; }");
	connect(expand_btn, &QPushButton::clicked, this, &ErrorDetailsWidget::toggleDetails);
	
	frame_layout->addWidget(expand_btn);
	frame_layout->addWidget(_error_list_w);
}

void ErrorDetailsWidget::toggleDetails()
{
	if (_error_list_w->isHidden())
	{
		_error_list_w->setHidden(false);
	}
	else
	{
		_error_list_w->setHidden(true);
	}
}
