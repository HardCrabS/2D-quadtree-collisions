#pragma once
#include "SFML/Graphics.hpp"

namespace Math
{
	inline float length(sf::Vector2f v)
	{
		return sqrt(v.x * v.x + v.y * v.y);
	}

	inline sf::Vector2f normalized(const sf::Vector2f& v)
	{
		return v / length(v);
	}
}