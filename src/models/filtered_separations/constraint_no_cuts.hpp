#pragma once

#include <memory>

#include <ghost/constraint.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

#include "data_zone.hpp"

using point = boost::geometry::model::d2::point_xy<int>;
using polygon = boost::geometry::model::polygon<point>;
using multi_point = boost::geometry::model::multi_point<point>;
using ring = boost::geometry::model::ring<point>;
using line = boost::geometry::model::linestring<point>;

class NoCuts : public ghost::Constraint
{
	std::shared_ptr<DataZone> _data;
	
public:
	NoCuts( const std::vector<ghost::Variable>& variables,
	        std::shared_ptr<ghost::AuxiliaryData> data );

	double required_error( const std::vector<ghost::Variable*>& variables ) const override;
};
