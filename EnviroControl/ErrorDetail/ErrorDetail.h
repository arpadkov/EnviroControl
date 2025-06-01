#pragma once

#include <QString>

class ErrorDetail
{
public:
	ErrorDetail(const QString& error_message) : _error_message(error_message)
	{
	};

	ErrorDetail(const char* error_message) : _error_message(error_message)
	{
	};

	QString getErrorMessage() const;

private:
	QString _error_message;
};