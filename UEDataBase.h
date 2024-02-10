#pragma once
#include <map>
#include <algorithm>
#include <vector>
#include <memory>
#include <stdexcept>

#include "UERecord.h"

class UEDataBase
{
protected:
	std::vector <std::shared_ptr<UERecord>> database;
	std::map<size_t, size_t> lookUpTbl;
	void updateLUT();

public:
	//write constructor & destructor

	const std::vector<std::shared_ptr<UERecord>>& readDB() const;
	std::vector<std::shared_ptr<UERecord>>& readWriteDB();

	void push_back(const UERecord& uer);

	std::shared_ptr<UERecord> look_up_m(const size_t& userID);
	std::shared_ptr<UERecord> look_up(const size_t& userID) const;

	const size_t size() const;
	void shuffle();

	const size_t removeUser(const size_t& userID);
};
