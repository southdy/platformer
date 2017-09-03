#ifndef GAME_AUDIO_COMPONENT_H
#define GAME_AUDIO_COMPONENT_H

#include "Component.h"

namespace game
{
    class PlayerComponent;

    /**
     * Listens for audible player messages and triggers playback of audio sources
     *
     * @script{ignore}
    */
    class AudioComponent : public gameobjects::Component
    {
    public:
        explicit AudioComponent();
        ~AudioComponent();
    protected:
        virtual void initialize() override;
        virtual void finalize() override;
        virtual bool onMessageReceived(gameobjects::Message *, int messageType) override;
        virtual void readProperties(gameplay::Properties & properties) override;
    private:
        AudioComponent(AudioComponent const &);

        void addAudioNode(std::string const & audioSourcePath);
        void playSoundEffect(std::string const & audioSourcePath);
        std::string _jumpAudioSourcePath;
        std::string _enemyDeathAudioSourcePath;
        std::string _playerDeathAudioSourcePath;
        std::map<std::string, gameplay::Node *> _audioNodes;
        PlayerComponent * _player;
    };
}

#endif
