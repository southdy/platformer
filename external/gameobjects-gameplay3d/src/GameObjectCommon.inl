namespace gameobjects
{
    template<typename NodeUserDataType>
    NodeUserDataType * getNodeUserData(gameplay::Node const * node, int id)
    {
        NodeUserDataType * data = nullptr;

        if(INodeUserData * dataInterface = static_cast<INodeUserData*>(node->getUserObject()))
        {
            if(dataInterface->getNodeUserDataId() == id)
            {
                data = static_cast<NodeUserDataType*>(dataInterface);
            }
        }

        return data;
    }

    template<>
    /** @script{ignore} */
    inline void setIfExists(gameplay::Properties & properties, std::string const & name, float & out)
    {
        if(properties.exists(name.c_str()))
        {
            out = properties.getFloat(name.c_str());
        }
    }

    template<>
    /** @script{ignore} */
    inline void setIfExists(gameplay::Properties & properties, std::string const & name, gameplay::Vector2 & out)
    {
        if(properties.exists(name.c_str()))
        {
            properties.getVector2(name.c_str(), &out);
        }
    }

    template<>
    /** @script{ignore} */
    inline void setIfExists(gameplay::Properties & properties, std::string const & name, bool & out)
    {
        if(properties.exists(name.c_str()))
        {
            out = properties.getBool(name.c_str());
        }
    }
}
