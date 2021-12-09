#pragma once

#include <ghost/objective.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using polygon = boost::geometry::model::polygon<point>;
using segment = boost::geometry::model::segment<point>;

class MinSeparationWidth : public ghost::Minimize
{
	polygon _contour;

public:
	MinSeparationWidth( const std::vector<ghost::Variable>& variables, const polygon& contour );

	double required_cost( const std::vector<ghost::Variable*>& variables ) const override;
};
