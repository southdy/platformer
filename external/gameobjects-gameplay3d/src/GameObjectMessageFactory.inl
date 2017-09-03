namespace gameobjects
{
    template<>
    inline int getMessageProperty(Message * message, int index)
    {
        return message->getInt(index);
    }

    template<>
    inline unsigned int getMessageProperty(Message * message, int index)
    {
        return message->getInt(index);
    }

    template<>
    inline bool getMessageProperty(Message * message, int index)
    {
        return message->getBoolean(index);
    }

    template<>
    inline float getMessageProperty(Message * message, int index)
    {
        return message->getFloat(index);
    }

    template<>
    inline char const * getMessageProperty(Message * message, int index)
    {
        return message->getString(index);
    }

    template<>
    inline void setMessageProperty(Message * message, int index, int value)
    {
        message->setInt(index, value);
    }

    template<>
    inline void setMessageProperty(Message * message, int index, unsigned int value)
    {
        message->setInt(index, value);
    }

    template<>
    inline void setMessageProperty(Message * message, int index, bool value)
    {
        message->setBoolean(index, value);
    }

    template<>
    inline void setMessageProperty(Message * message, int index, float value)
    {
        message->setFloat(index, value);
    }

    template<>
    inline void setMessageProperty(Message * message, int index, char const* value)
    {
        message->setString(index, value);
    }
}
