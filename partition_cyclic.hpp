
#ifndef ML__ALGORITHM__PARTITION_CYCLIC_HPP
#define ML__ALGORITHM__PARTITION_CYCLIC_HPP

#include <cassert>

namespace ml {
namespace algorithm {


/// Implementation of the "Cyclic partition" algorithm.
template< typename BidIt, typename Pred >
inline BidIt partition_cyclic(
		BidIt left, BidIt right, Pred pred )
{
	// Search for the first value from left, not on its place
	while ( left != right && pred( *left ) )
		++left;
	if ( left == right )
		return right;  // All values satisfy the predicate
	assert( ! pred( *left ) );
	// First value from left, not on its place is found
	const auto tmp = *left;  // The only copy to 'tmp' variable
	--right;  // Move on first value from the right
	while ( true ) {
		// Step 1: Must place something in '*left'
		while ( left != right && ! pred( *right ) )
			--right;
		if ( left == right )  // Check if the scans are completed
			break;
		assert( pred( *right ) );
		*(left++) = *right;
		// Step 2: Must place something in '*right'
		while ( left != right && pred( *left ) )
			++left;
		if ( left == right )  // Check if the scans are completed
			break;
		assert( ! pred( *left ) );
		*(right--) = *left;
	}
	// Here the scans are completed
	assert( left == right );
	*right = tmp;  // The only copy from 'tmp' variable
	return right;
}


}
}

#endif
