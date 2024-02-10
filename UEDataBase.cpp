#include "UEDataBase.h"
#include <random>
#include "Simulator.h"
#include "ErrorTracer.h"


void UEDataBase::updateLUT()
{
	lookUpTbl.clear();
	auto index = size_t{ 0 };
	for (auto& record : this->database)
		lookUpTbl.insert(std::pair<size_t, size_t>{(*record).userID, index++});
}

const std::vector<std::shared_ptr<UERecord>>& UEDataBase::readDB() const
{
	return this->database;
}

std::vector<std::shared_ptr<UERecord>>& UEDataBase::readWriteDB()
{
	return this->database;
}

void UEDataBase::push_back(const UERecord& uer)
{
	this->database.push_back(std::make_shared<UERecord>(uer));
	updateLUT();
}

//FLAG obv reason
std::shared_ptr<UERecord> UEDataBase::look_up_m(const size_t& userID)
{
	try
	{
		auto it = this->lookUpTbl.find(userID);
		auto ptr = std::shared_ptr<UERecord>{};
		if (it != this->lookUpTbl.end())
			ptr = database.at((*it).second);
		return ptr;
	}
	catch (...)
	{
		return std::shared_ptr<UERecord>{};
	}

}

//FLAG obv reason
std::shared_ptr<UERecord> UEDataBase::look_up(const size_t& userID) const
{
	try
	{
		auto it = this->lookUpTbl.find(userID);
		auto ptr = std::shared_ptr<UERecord>{};
		if (it != this->lookUpTbl.end())
			ptr = database[(*it).second];
		return ptr;
	}
	catch (...)
	{
		return std::shared_ptr<UERecord>{};
	}

}

const size_t UEDataBase::size() const
{
	return this->database.size();
}

void UEDataBase::shuffle()
{
	std::shuffle(this->database.begin(), this->database.end(), Simulator::getRandNumEngine());
	updateLUT();
}

//FLAG fix error handle
const size_t UEDataBase::removeUser(const size_t& userID)
{
	for (auto it = this->database.begin(); it != this->database.end(); it++)
	{
		if ((*(*it)).userID == userID)
		{
			int antTransInfo = (*(*it)).antenna;
			this->database.erase(it);
			updateLUT();
			return antTransInfo;
		}
	}
	return (size_t)ErrorTracer::error("\nUNABLE TO REMOVE USER in UEDataBase::removeUser(const int& userID)");
}

