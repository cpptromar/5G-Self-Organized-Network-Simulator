#include <cmath>
#include <numeric>

#include "Transceivers.h"
#include "UserEquipment.h"
#include "Simulator.h"
#include "ErrorTracer.h"



Transceiver::Transceiver(const size_t& trID, const Coord<float>& loc, const float& aTheta)
{
	this->trLoc = Coord<float>{ loc };
	this->tranceiverID = trID;
	this->theta = aTheta;
	this->antTheta = aTheta;
}

const size_t& Transceiver::getTransceiverID() const
{
	return this->tranceiverID;
}

const Coord<float>& Transceiver::getLoc() const
{
	return trLoc;
}

const float& Transceiver::getAntTheta() const
{
	return this->antTheta;
}

const float& Transceiver::getTheta() const
{
	return this->antTheta;
}


TransceiverList::TransceiverList()
{
	this->transList = std::vector<Transceiver>(Simulator::getNumberOfTransceivers());

	this->availableTransceivers = std::vector<size_t>(Simulator::getNumberOfTransceivers());
	std::iota(std::begin(availableTransceivers), std::end(availableTransceivers), 0);

	this->userTransPairing = std::vector<std::pair<size_t, size_t>>{};
	this->userTransPairing.reserve(Simulator::getNumberOfTransceivers());

}

TransceiverList::TransceiverList(const TransceiverList& trl)
{
	this->transList = std::vector<Transceiver>{trl.transList};

	this->availableTransceivers = std::vector<size_t>{ trl.availableTransceivers };

	this->userTransPairing = std::vector<std::pair<size_t, size_t>>{ trl.userTransPairing };

}

TransceiverList::TransceiverList(TransceiverList&& trl) noexcept
{
	this->transList = std::move(trl.transList);
	this->availableTransceivers = std::move(trl.availableTransceivers);
	this->userTransPairing = std::move(trl.userTransPairing);
}


Transceiver& TransceiverList::operator[](size_t x)
{
	if (x < this->transList.size() && x >= 0)
	{
		return this->transList[x];
	}
	else
	{
		return this->transList[0];
	}
}

const Transceiver& TransceiverList::operator[](size_t x) const
{
	if (x < this->transList.size() && x >= 0)
	{
		return this->transList[x];
	}
	else
	{
		return this->transList[0];
	}
}

auto TransceiverList::getTransceivers() const -> const decltype(this->transList)&
{
	return this->transList;
}

auto TransceiverList::getTransceivers_m() -> decltype(transList)&
{
	return transList;
}

auto TransceiverList::getUserTransPairings() const -> const decltype(this->userTransPairing)&
{
	return this->userTransPairing;
}

auto TransceiverList::getAvailableTransceivers() const -> const decltype(this->availableTransceivers)&
{
	return this->availableTransceivers;
}

std::pair<bool,size_t> TransceiverList::addUser(const size_t& user)
{
	if (this->availableTransceivers.size() > 0)
	{
		size_t assignedTransceiver = *(this->availableTransceivers.end() - 1);
		this->availableTransceivers.pop_back();
		this->userTransPairing.push_back(std::pair<size_t, size_t>( user, assignedTransceiver ));
		return std::make_pair(true, assignedTransceiver);
	}
	else
		return std::make_pair(false, 0);

}

const bool TransceiverList::removeUser(const size_t& user)
{
	for (auto uetp = userTransPairing.begin(); uetp != userTransPairing.end(); uetp++)
	{
		if ((*uetp).first == user)
		{
			this->availableTransceivers.push_back((*uetp).second);
			this->userTransPairing.erase(uetp);
			return true;
		}
	}
	return false;
}

const size_t TransceiverList::numberOfTransceiversUsed() const
{
	return this->userTransPairing.size();
}

const size_t TransceiverList::getUserID(const size_t& tr) const
{
	if (tr < this->userTransPairing.size())
		return this->userTransPairing[tr].first;
	else 
		return static_cast<size_t>(ErrorTracer::error("\nATTEMPTED TO ADD TOO MANY USERS TO ANTENNA in TransceiverList::getUserID(const size_t& tr)"));
}

const bool TransceiverList::isFull() const
{
	return (availableTransceivers.size() <= 0);
}
