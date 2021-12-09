#pragma once

#include <memory>

#include <ghost/objective.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/linestring.hpp>

#include "data_zone.hpp"

class MinSeparationWidth : public ghost::Minimize
{
	std::shared_ptr<DataZone> _data;
	
public:
	MinSeparationWidth( const std::vector<ghost::Variable>& variables,
	                    std::shared_ptr<ghost::AuxiliaryData> data );
	
	double required_cost( const std::vector<ghost::Variable*>& variables ) const override;
};
