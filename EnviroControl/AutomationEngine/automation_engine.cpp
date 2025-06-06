#include "AutomationEngine.h"
#include "WeatherStation.h"
#include "DeviceStateManager.h"
#include "RulesProcessor.h"

namespace Automation
{

namespace
{
void addCircularBufferData(std::vector<WeatherData>& buffer, const WeatherData& data, int max_size)
{
	buffer.insert(buffer.begin(), data);
	if (buffer.size() > max_size)
		buffer.pop_back();
}
}

AutomationEngine::AutomationEngine(QObject* parent) : QObject(parent)
{
	_calc_timer = new QTimer(this);
	connect(_calc_timer, &QTimer::timeout, this, &AutomationEngine::onCalcTimeout);
	_calc_timer->start();
}

void AutomationEngine::setManualMode()
{
	_calc_timer->stop();
}

void AutomationEngine::onWeatherStationData(const WeatherData& weather_data)
{
	addCircularBufferData(_weather_data_history, weather_data, _weather_data_history_length);
}

void AutomationEngine::onCalcTimeout()
{
	if (_weather_data_history.empty())
		return;

	const auto& calculated_states = RulesProcessor::calculateDeviceStates({}, _weather_data_history);
	Q_EMIT deviceStatesUpdated(calculated_states);
}

}