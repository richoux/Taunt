#include <string>
#include <sstream>
#include <fstream>
#include <list>
#include <iterator>

#include "constraint_one_cluster_per_region.hpp"

OneClusterPerRegion::OneClusterPerRegion( const std::vector<ghost::Variable>& variables,
                                          std::shared_ptr<ghost::AuxiliaryData> data )
	: Constraint( variables ),
	  _data( std::dynamic_pointer_cast<DataZone>( data ) )
{ }

double OneClusterPerRegion::required_error( const std::vector<ghost::Variable*>& variables ) const
{
	int error = 0;
	std::vector<point> cluster_centers;
	for( auto& resource : _data->resources )
	{
		point center;
		boost::geometry::centroid( resource, center );
		cluster_centers.push_back( center );
	}

	for( auto& var : variables )
		if( var->get_value() == 1 )
			for( auto& zone : _data->zones )
				if( boost::geometry::covered_by( _data->separations[var->get_id()], zone ) )
				{
					bool found_center = false;
				
					for( auto& center : cluster_centers )
						if( boost::geometry::covered_by( center, zone ) )
							if( found_center )
								++error;
							else
								found_center = true;
				
					if( !found_center )
						++error;
				}
	
	return error;
}
