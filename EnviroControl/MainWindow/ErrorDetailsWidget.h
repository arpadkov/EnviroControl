#pragma once

#include <QtWidgets/QWidget>

class QListWidget;

class ErrorDetailsWidget : public QWidget
{
	Q_OBJECT

public:
	ErrorDetailsWidget(QWidget* parent = nullptr);
	~ErrorDetailsWidget();

public Q_SLOTS:
	void addError(const QString& error);

private:
	void initLayout();
	void toggleDetails();

	QListWidget* _error_list_w;
};