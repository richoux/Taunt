#pragma once

#include <ghost/constraint.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using polygon = boost::geometry::model::polygon<point>;
using multi_point = boost::geometry::model::multi_point<point>;
using ring = boost::geometry::model::ring<point>;
using line = boost::geometry::model::linestring<point>;

class NoCuts : public ghost::Constraint
{
	polygon _contour;
	std::vector<multi_point> _resources;

public:
	NoCuts( const std::vector<ghost::Variable>& variables,
	        const polygon& contour,
	        const std::vector<multi_point>& resources );

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;
};
