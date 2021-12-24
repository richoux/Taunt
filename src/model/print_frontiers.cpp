#include <iomanip>

#include "model/print_frontiers.hpp"

PrintFrontiers::PrintFrontiers( const std::vector<line>& separations )
	: _separations( separations )
{ }
	
std::stringstream PrintFrontiers::print_candidate( const std::vector<ghost::Variable>& variables ) const
{
	std::stringstream ss;

	for( int i = 0 ; i < static_cast<int>( variables.size() ) ; ++i )
		if( variables[i].get_value() == 1 )
			ss << "Separation " << boost::geometry::dsv( _separations[i] ) << "\n";
	
	return ss;
}
