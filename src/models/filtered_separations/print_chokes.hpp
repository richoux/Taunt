#pragma once

#include <vector>

#include <ghost/print.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using line = boost::geometry::model::linestring<point>;

class PrintChokes : public ghost::Print
{
	std::vector<line> _separations;
	
public:
	PrintChokes( const std::vector<line>& separations );
	
	std::stringstream print_candidate( const std::vector<ghost::Variable>& variables ) const override;
};	
