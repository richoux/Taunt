#include "objective_min_separation_width.hpp"

MinSeparationWidth::MinSeparationWidth( const std::vector<ghost::Variable>& variables, std::shared_ptr<ghost::AuxiliaryData> data )
	: Minimize( variables, "Minimize Separation Width" ),
	  _data( std::dynamic_pointer_cast<DataZone>( data ) )
{}

double MinSeparationWidth::required_cost( const std::vector<ghost::Variable*>& variables ) const
{
	double total_width = 0.0;
	
	for( size_t i = 0 ; i < variables.size() - 1 ; ++i )
		if( variables[i]->get_value() == 1  )
			total_width += _data->separation_lengths[i];

	return total_width;
}
