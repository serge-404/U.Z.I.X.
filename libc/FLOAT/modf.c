#include	"math.h"

double modf(double x, double *intf)
{
	*intf=floor(x);
	return x-(*intf);
}
