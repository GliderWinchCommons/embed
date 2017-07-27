/******************************************************************************
* File Name          : lltoflt.c
* Date First Issued  : 07/25/2017
* Board              : Discovery F4
* Description        : long long to float
*******************************************************************************/
/*
 If the high word of the long long is zero return a float of zero, otherwise
   make the long long positive, 
   count the number of leading zeros in the high word, 
   left shift the long long and use the high word for the float 
   after restoring the sign.

Input -> output results of a test of CLZ in the inline assembly--
     ~0 -> 0
      0 -> 32
      1 -> 31
(1<<30) -> 1

*/

/* **************************************************************************************
 * float lltoflt(long long x);
 * @brief	: Scale long long if necessary to fit into float
 * @param	: x - input long long
 * @return	: float
 * ************************************************************************************** */
float lltoflt(long long x)
{
	volatile int clzDst;	// CLZ instruction destination
	float fx;
	union LLN
	{
		long long ll;
		volatile int n[2];
	};
	volatile union LLN tmp;

	tmp.ll = x;	// Working copy (x retains sign)

	if (tmp.n[1] == 0) // High order word zero?
	{ // Here, no need to fiddle with shifting
		fx = tmp.n[0];	// Convert to float
 		return fx;	// Quick return;
	}

	/* Left justify long long (to bit 30 to allow for negative) */

	if (x < 0) tmp.ll = -tmp.ll;	// Make working copy positive
	
	/* Inline assembly of CLZ instruction */
/*
	// Count number of leading zeroes in high word
	__asm__ volatile ("CLZ %1, %0\n\t"
	: "=r" (clzDst)
	: "r" (tmp.n[1]));
*/
	clzDst = __builtin_clz(tmp.n[1]);

	tmp.ll = (tmp.ll << (clzDst-1));     // Remove leading zeroes
	if (x < 0) tmp.n[1] = -tmp.n[1]; // Restore sign
	fx = tmp.n[1];		// Convert to float
	fx *= 1<<(33-clzDst); 	// Scale for amount of shift
	return fx;
}

