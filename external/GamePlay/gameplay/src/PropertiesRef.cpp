#include "PropertiesRef.h"
#include "Properties.h"

namespace gameplay
{
    PropertiesRef::PropertiesRef(Properties * properties)
        : _properties(properties)
    {
    }

    PropertiesRef::~PropertiesRef()
    {
        SAFE_DELETE(_properties);
    }

    Properties * PropertiesRef::get() const
    {
        return _properties;
    }

    PropertiesRef * PropertiesRef::create(Properties * properties)
    {
        return new PropertiesRef(properties);
    }
}
