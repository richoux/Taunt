#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <iterator>

#include "constraint_minimal_region_area.hpp"

MinRegionArea::MinRegionArea( const std::vector<ghost::Variable>& variables,
                              std::shared_ptr<ghost::AuxiliaryData> data,
                              int number_separations )
	: Constraint( variables ),
	  _data( std::dynamic_pointer_cast<DataZone>( data ) ),
	  _target_area( std::max( 300, static_cast<int>( boost::geometry::area( _data->contour.outer() ) / ( number_separations + 4 ) ) ) ) // say we need 2 separations, so target areas should be at least 1/6th of the total area
{ }

double MinRegionArea::required_error( const std::vector<ghost::Variable*>& variables ) const
{
	int error = 0;

	for( auto& var : variables )
		if( var->get_value() == 1 )
			for( auto& zone : _data->zones )
				if( boost::geometry::covered_by( _data->separations[var->get_id()], zone ) && boost::geometry::area( zone ) < _target_area )
					++error;

	return error;
}
