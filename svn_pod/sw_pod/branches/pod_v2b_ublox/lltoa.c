/*******************************************************************************
* File Name          : lltoa.c
* Date First Issued  : 05/22/2017
* Board              : 
* Description        : Convert long long to ascii, given some base
*******************************************************************************/



/******************************************************************************
 * char* lltoa(char *p, long long val, int base);
 * @brief	: convert long long to ascii
 * @param	: p = pointer to buffer (should be > 62)
 * @param	: val = long long to be convert to ascii
 * @param	: base = base for conversion, e.g. 10
 * @return	: pointer to first char of string
 ******************************************************************************/
char* lltoa(char *p, long long val, int base)
{
    int i = 62;		// Longest result
    *(p+i+1) = 0;	// String termination
    int sign = (val < 0);

    if(sign) val = -val;

    if(val == 0) return "0";

    for(; val && i ; --i, val /= base) 
    {
        *(p+i) = "0123456789abcdef"[val % base];
    }

    if(sign) 
    {
        *(p+i) = '-';i--;
    }
    return p+i+1;
}

