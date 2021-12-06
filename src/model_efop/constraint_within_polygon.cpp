#include "constraint_within_polygon.hpp"

WithinPolygon::WithinPolygon( const std::vector<ghost::Variable>& variables, const polygon& contour )
	: Constraint( variables ),
	  _contour( contour )
{ }

double WithinPolygon::required_error( const std::vector<ghost::Variable*>& variables ) const
{
	std::vector< bool > processed( variables.size(), false );
	int error = 0;
	
	for( size_t i = 0 ; i < variables.size() - 1 ; ++i )
	{
		if( variables[i]->get_value() > 0 && !processed[i] )
		{
			processed[i] = true;
			for( size_t j = i + 1 ; j < variables.size() ; ++j )
				if( variables[j]->get_value() == variables[i]->get_value() && !processed[j] )
				{
					processed[j] = true;
					ring ring_line{{ _contour.outer()[i], _contour.outer()[j] }};
					if( boost::geometry::within( ring_line, _contour.outer() ) )
						++error;
				}
		}
	}

	return error;
}
