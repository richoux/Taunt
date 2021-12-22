#include "model/objective_min_separation_width.hpp"

MinSeparationWidth::MinSeparationWidth( const std::vector<ghost::Variable>& variables, const std::vector<line>& separations )
	: Minimize( variables, "Minimize Separation Width" ),
	  _separation_lengths( std::vector<double>( separations.size() ) )
{
	for( size_t i = 0 ; i < separations.size() ; ++i )
		_separation_lengths[i] = boost::geometry::length( separations[i] );
}

double MinSeparationWidth::required_cost( const std::vector<ghost::Variable*>& variables ) const
{
	double total_width = 0.0;
	
	for( size_t i = 0 ; i < variables.size() ; ++i )
		if( variables[i]->get_value() == 1  )
			total_width += _separation_lengths[i];

	return total_width;
}
