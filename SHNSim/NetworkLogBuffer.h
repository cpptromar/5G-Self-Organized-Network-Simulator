#pragma once
#include <stdint.h> // needed to use uint32_t type in Linux
#include <cstddef>
#include "UELogData.h"
#include <list>

class NetworkLogBuffer 
{
protected:
	size_t windowSizeInTicks;
	std::list<TickData> TickBuffer;
public:
    NetworkLogBuffer();

	auto begin() -> decltype(this->TickBuffer.begin());
	auto end() -> decltype(this->TickBuffer.end());
	auto begin() const -> const decltype(this->TickBuffer.begin());
	auto end() const-> const decltype(this->TickBuffer.end());
	void push_back(const TickData&);
	size_t size() const;
	const TickData& getLastTick() const;

	void setWindowSize(const size_t& numTicks);
};
