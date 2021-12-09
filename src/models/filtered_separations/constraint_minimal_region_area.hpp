#pragma once

#include <memory>

#include <ghost/constraint.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

#include "data_zone.hpp"

using point = boost::geometry::model::d2::point_xy<int>;
using line = boost::geometry::model::linestring<point>;
using ring = boost::geometry::model::ring<point>;
using polygon = boost::geometry::model::polygon<point>;

class MinRegionArea : public ghost::Constraint
{
	std::shared_ptr<DataZone> _data;
	int _target_area;
	
public:
	MinRegionArea( const std::vector<ghost::Variable>& variables,
	               std::shared_ptr<ghost::AuxiliaryData> data,
	               int number_separations );

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;
};
