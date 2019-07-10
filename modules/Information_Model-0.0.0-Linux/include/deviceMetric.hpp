#ifndef METRIC_H
#define METRIC_H
#include "deviceElement.hpp"
#include "valueDataType.hpp"
#include <string>

class DeviceMetric : public DeviceElement
{
public:
	
private:
	ValueType::ValueDataType valueType;

public:
	DeviceMetric(std::string refId, std::string name, std::string desc, ValueType::ValueDataType valueType);
	ValueType::ValueDataType getMetricValueType();
};


class WriteableMetric : public DeviceMetric
{
public:
	WriteableMetric(std::string refId, std::string name, std::string desc, ValueType::ValueDataType valueType);
	//void updateSensor(std::string updateValues);
	void CheckIfWriteableMetric(std::string updateValues);
};


class ObservableMetric : public DeviceMetric
{
private:
	std::string registration_id;
	static int incrementId;
public:
	
	ObservableMetric(std::string refId, std::string name, std::string desc, ValueType::ValueDataType valueType);

	void registerMetric();

	std::string getRegistrationId();


};

#endif