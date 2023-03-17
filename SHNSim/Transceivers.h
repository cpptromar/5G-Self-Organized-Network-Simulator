#pragma once
#include <vector>
#include <stdint.h> // needed to use uint32_t on Linux
#include <cstddef>
#include "Coord.h"

class Transceiver
{
protected:
	size_t tranceiverID;
	Coord<float> trLoc;
	float theta;
	float antTheta;

public:
	Transceiver() = default;
	Transceiver(const size_t& trID, const Coord<float>& loc, const float& antTheta);
	Transceiver(const Transceiver&) = default;
	Transceiver(Transceiver&&) = default;
	Transceiver& operator=(const Transceiver&) = default;

	const size_t& getTransceiverID() const;
	const Coord<float>& getLoc() const;
	const float& getAntTheta() const;
	const float& getTheta() const;
};

class TransceiverList
{
protected:
	std::vector<Transceiver> transList;
	std::vector<std::pair</*userID*/size_t, /*TransID*/size_t>> userTransPairing;
	std::vector<size_t> availableTransceivers;

public:
	TransceiverList();
	TransceiverList(const TransceiverList&);
	TransceiverList(TransceiverList&&) noexcept;
	TransceiverList& operator=(const TransceiverList&) = default;
	Transceiver& operator[](size_t x);
	const Transceiver& operator[](size_t x) const;

	auto getTransceivers() const -> const decltype(transList)&;
	auto getTransceivers_m() -> decltype(transList)&;
	auto getUserTransPairings() const -> const decltype(userTransPairing)&;
	auto getAvailableTransceivers() const -> const decltype(availableTransceivers)&;

	std::pair<bool, size_t> addUser(const size_t& user);
	const bool removeUser(const size_t& user);
	const size_t numberOfTransceiversUsed() const;
	const size_t getUserID(const size_t& tr) const;
	const bool isFull() const;
};
