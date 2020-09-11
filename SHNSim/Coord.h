#pragma once
#include <utility>

template <class T>
struct Coord
{
	T x, y;

	Coord()
	{
		this->x = 0;
		this->y = 0;
	};

	Coord(const T& X, const T& Y)
	{
		this->x = X;
		this->y = Y;
	}

	Coord(const Coord& l)
	{
		this->x = l.x;
		this->y = l.y;
	}

	Coord(Coord&& l) noexcept
	{
		this->x = std::exchange(l.x, static_cast<T>(0));
		this->y = std::exchange(l.y, static_cast<T>(0));
	}

	Coord& operator=(const Coord&) = default;

	~Coord() = default;
};