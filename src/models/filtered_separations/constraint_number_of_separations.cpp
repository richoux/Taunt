#include "constraint_number_of_separations.hpp"

NumberSeparations::NumberSeparations( const std::vector<ghost::Variable>& variables, int number_separations )
	: Constraint( variables ),
	  _number_separations( number_separations )
{ }

double NumberSeparations::required_error( const std::vector<ghost::Variable*>& variables ) const
{
	int error = 0;

	for( int separation = 1 ; separation <= _number_separations; ++separation )
		error += std::abs( 2 - std::count_if( variables.begin(), variables.end(), [&](auto& v){ return v->get_value() == separation; } ) );

	return error;	
}
