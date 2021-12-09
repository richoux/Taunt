#pragma once

#include <ghost/constraint.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using line = boost::geometry::model::linestring<point>;
using ring = boost::geometry::model::ring<point>;
using polygon = boost::geometry::model::polygon<point>;

class MinRegionArea : public ghost::Constraint
{
	polygon _contour;
	int _number_separations;
	int _zone_area;
	int _target_area;
	
public:
	MinRegionArea( const std::vector<ghost::Variable>& variables,
	               const polygon& contour,
	               int number_separations );

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;
};
