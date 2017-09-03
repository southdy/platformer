#ifndef PROPERTIES_REF
#define PROPERTIES_REF

#include "Ref.h"

namespace gameplay
{
    class Properties;

    /** @script{ignore} */
    class PropertiesRef : public Ref
    {
    public:
        ~PropertiesRef();
        Properties * get() const;
        static PropertiesRef * create(Properties * properties);
    private:
        PropertiesRef(Properties * properties);

        Properties * _properties;
    };
}

#endif
