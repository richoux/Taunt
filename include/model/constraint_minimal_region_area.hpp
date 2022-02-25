#pragma once

#include <memory>

#include <ghost/constraint.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

using point = boost::geometry::model::d2::point_xy<int>;
using line = boost::geometry::model::linestring<point>;
using ring = boost::geometry::model::ring<point>;
using polygon = boost::geometry::model::polygon<point>;

class MinRegionArea : public ghost::Constraint
{
	polygon _polygon;
	std::vector<line> _separations;
	int _target_area;

	bool on_the_border( const line& separation, const ring& zone ) const;
	bool is_in( const line& separation, const ring& zone ) const;
	
public:
	MinRegionArea( const std::vector<ghost::Variable>& variables,
	               const polygon& polygon,
	               const std::vector<line>& separations,
	               int number_separations );

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;
};
