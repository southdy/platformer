#ifndef GAMEOBJECT_COMMON_H
#define GAMEOBJECT_COMMON_H

#include <array>
#include "GameObjectMessageFactory.h"
#include "Node.h"
#include "Ref.h"

namespace gameobjects
{
    #define GAMEOBJECT_ASSERT(expression, ...) if (!(expression)) GP_ERROR(__VA_ARGS__)
    #define GAMEOBJECT_ASSERTFAIL(...) GAMEOBJECT_ASSERT(false, __VA_ARGS__)

    class GameObjectCallbackHandler : public gameplay::Ref
    {
    public:
        virtual gameplay::Properties * getProperties(const char * url) = 0;
    };

    /**
     * You must use this interface to define a unique type id you intend to attach gameplay::Ref user objects to nodes
     *
     * @script{ignore}
    */
    class INodeUserData : public gameplay::Ref
    {
    public:
        virtual int getNodeUserDataId() const = 0;
    };

    template<typename UnImplementedType>
    /** @script{ignore} */
    inline void setIfExists(gameplay::Properties & properties, std::string const & name, UnImplementedType & out);

    template<>
    /** @script{ignore} */
    inline void setIfExists(gameplay::Properties & properties, std::string const & name, float & out);

    template<>
    /** @script{ignore} */
    inline void setIfExists(gameplay::Properties & properties, std::string const & name, gameplay::Vector2 & out);

    template<>
    /** @script{ignore} */
    inline void setIfExists(gameplay::Properties & properties, std::string const & name, bool & out);

    #define GAMEOBJECTS_MESSAGE_0(Name) GAMEOBJECTS_MESSAGE_0_INTERNAL(Name)
    #define GAMEOBJECTS_MESSAGE_1(Name, T1, N1) GAMEOBJECTS_MESSAGE_1_INTERNAL(Name, T1, N1)
    #define GAMEOBJECTS_MESSAGE_2(Name, T1, N1, T2, N2) GAMEOBJECTS_MESSAGE_2_INTERNAL(Name, T1, N1, T2, N2)
    #define GAMEOBJECTS_MESSAGE_3(Name, T1, N1, T2, N2, T3, N3) GAMEOBJECTS_MESSAGE_3_INTERNAL(Name, T1, N1, T2, N2, T3, N3)
    #define GAMEOBJECTS_MESSAGE_4(Name, T1, N1, T2, N2, T3, N3, T4, N4) GAMEOBJECTS_MESSAGE_4_INTERNAL(Name, T1, N1, T2, N2, T3, N3, T4, N4)

    #define GAMEOBJECTS_MESSAGE_TYPES_BEGIN() GAMEOBJECTS_MESSAGE_TYPES_BEGIN_INTERNAL()
    #define GAMEOBJECTS_MESSAGE_TYPE(Name) GAMEOBJECTS_MESSAGE_TYPE_INTERNAL(Name)
    #define GAMEOBJECTS_MESSAGE_TYPES_END() GAMEOBJECTS_MESSAGE_TYPES_END_INTERNAL()
}

#include "GameObjectCommon.inl"

#endif
