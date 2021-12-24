#pragma once

#include <vector>

#include "positions.hpp"

namespace taunt
{
	class polygon : public std::vector<position>
	{
		 const double get_area() const;
		 const double get_perimeter() const;
		 const position get_center() const;
		 position get_nearest_point( const position& p ) const;
	};
}
