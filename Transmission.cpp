#include "Transmission.h"

Transmission::Transmission(const size_t& sendr, const size_t& destin, const size_t& an, const size_t& t, const uint32_t& numB)
{
	this->sender = sendr;
	this->destination = destin;
	this->ant = an;
	this->tr = t;
	this->data = numB;
}