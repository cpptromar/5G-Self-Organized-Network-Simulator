#include "NetworkLogBuffer.h"

NetworkLogBuffer::NetworkLogBuffer()
{
	this->TickBuffer = std::list<TickData>();
}

auto NetworkLogBuffer::begin() -> decltype(this -> TickBuffer.begin())
{
	return this->TickBuffer.begin();
}

auto NetworkLogBuffer::end() -> decltype(this -> TickBuffer.end())
{
	return this->TickBuffer.end();
}

auto NetworkLogBuffer::begin() const -> const decltype(this->TickBuffer.begin())
{
	return this->TickBuffer.begin();
}

auto NetworkLogBuffer::end() const -> const decltype(this->TickBuffer.end())
{
	return this->TickBuffer.end();
}

void NetworkLogBuffer::setWindowSize(const size_t& bufSize)
{
	this->windowSizeInTicks = bufSize;
}

void NetworkLogBuffer::push_back(const TickData& td)
{
	this->TickBuffer.push_back(td);
	if (TickBuffer.size() > windowSizeInTicks)
		TickBuffer.pop_front();
}

size_t NetworkLogBuffer::size() const
{
	return TickBuffer.size();
}

const TickData& NetworkLogBuffer::getLastTick() const
{
	return TickBuffer.back();
}
