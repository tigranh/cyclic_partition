
#ifndef ML__ALGORITHM__PARTITION_HOARE_HPP
#define ML__ALGORITHM__PARTITION_HOARE_HPP

#include <iterator>
#include <cassert>

namespace ml {
namespace algorithm {


/// Implementation of sequence partitioning by Hoare scheme.
template< typename RanIt, typename Pred >
inline RanIt partition_hoare(
		RanIt left, RanIt right, Pred pred )
{
	typename std::iterator_traits< RanIt >::value_type tmp;
	--right;  // Move on first value from the right
	while ( true ) {
		// Advance from left, until meeting value "not at proper place"
		while ( left < right && pred(*left) )
			++left;
		// Advance from right, until meeting value "not at proper place"
		while ( left < right && ! pred(*right) )
			--right;
		// Check if the scans have completed
		if ( left >= right )
			break;
		// Found 2 values, not at proper places
		assert( ! pred(*left) );
		assert( pred(*right) );
		tmp = *left;  // Swap them
		*(left++) = *right;
		*(right--) = tmp;
	}
	// Here the scans have completed
	assert( left == right || left == right + 1 );
	if ( left == right && pred(*left) )
		return left + 1;
	else
		return left;
}


}
}

#endif
