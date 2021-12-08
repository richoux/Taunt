#pragma once

#include <vector>
#include <ghost/print.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using polygon = boost::geometry::model::polygon<point>;

class PrintChokes : public ghost::Print
{
	polygon _contour;
	
public:
	PrintChokes( const polygon& contour );
	
	std::stringstream print_candidate( const std::vector<ghost::Variable>& variables ) const override;
};	
