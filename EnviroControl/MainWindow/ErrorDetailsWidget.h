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
	void onErrorOccurred(const QString& error);

private:
	void initLayout();
	void addError(const QString& error);
	void toggleDetails();

	QListWidget* _error_list_w;
};