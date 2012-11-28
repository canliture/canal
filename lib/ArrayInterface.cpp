#include "ArrayInterface.h"
#include "Domain.h"

namespace Canal {
namespace Array {

Domain *
Interface::getValue(const Domain &offset) const
{
    Domain *result = NULL;
    std::vector<Domain*> items(getItem(offset));
    std::vector<Domain*>::const_iterator it = items.begin(),
        itend = items.end();

    for (; it != itend; ++it)
    {
        if (!result)
            result = (*it)->clone();
        else
            result->join(**it);
    }

    return result;
}

} // namespace Array
} // namespace Canal
