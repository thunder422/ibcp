#include "rpnlist.h"

RpnList::~RpnList(void)
{
}


// function to clear all of the RPN items from the list
void RpnList::clear(void)
{
	while (!isEmpty())
	{
		// delete to free the RPN item that was in the list
		delete takeLast();
	}
}
