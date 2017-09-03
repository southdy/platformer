#include "AudioComponent.h"

#include "AudioSource.h"
#include "Common.h"
#include "ProfilerController.h"
#include "GameObjectController.h"
#include "Messages.h"
#include "PlayerComponent.h"

namespace game
{
    AudioComponent::AudioComponent()
        : _player(nullptr)
    {
    }

    AudioComponent::~AudioComponent()
    {
    }

    void AudioComponent::initialize()
    {
        addAudioNode(_jumpAudioSourcePath);
        addAudioNode(_enemyDeathAudioSourcePath);
        addAudioNode(_playerDeathAudioSourcePath);
    }

    void AudioComponent::finalize()
    {
        for(auto & pair : _audioNodes)
        {
            gameplay::Node * node = pair.second;

            if(node->getParent())
            {
                node->getParent()->removeChild(node);
            }

            GAME_ASSERT_SINGLE_REF(node);
            SAFE_RELEASE(node);
        }

        SAFE_RELEASE(_player);
    }

    void AudioComponent::addAudioNode(std::string const & audioSourcePath)
    {
        PROFILE();
        gameplay::Node * node = gameplay::Node::create(audioSourcePath.c_str());
        gameplay::AudioSource * audioSource = nullptr;
        {
            STALL_SCOPE();
            audioSource = gameplay::AudioSource::create(audioSourcePath.c_str());
        }
        node->setAudioSource(audioSource);
        // Play it silently so that it performs lazy initialisation
        audioSource->setGain(0.0f);
        audioSource->play();
        SAFE_RELEASE(audioSource);
        _audioNodes[audioSourcePath] = node;
    }

    void AudioComponent::playSoundEffect(std::string const & audioSourcePath)
    {
        gameplay::Node * audioNode = _audioNodes[audioSourcePath];
        audioNode->setTranslation(_player->getNode()->getTranslation());
        gameplay::AudioSource * source = audioNode->getAudioSource();
        source->setGain(1.0f);
        source->play();
    }

    bool AudioComponent::onMessageReceived(gameobjects::Message *, int messageType)
    {
        switch (messageType)
        {
            case(Messages::Type::LevelLoaded):
            {
                _player = getParent()->getComponentInChildren<PlayerComponent>();
                _player->addRef();

                for(auto & audioNodePair : _audioNodes)
                {
                    _player->getParent()->getNode()->addChild(audioNodePair.second);
                }

                return true;
            }
            case(Messages::Type::LevelUnloaded):
                SAFE_RELEASE(_player);
                return true;
            case(Messages::Type::PlayerJump):
                playSoundEffect(_jumpAudioSourcePath);
                return false;
            case(Messages::Type::EnemyKilled):
                playSoundEffect(_enemyDeathAudioSourcePath);
                return false;
            case(Messages::Type::PlayerReset):
                playSoundEffect(_playerDeathAudioSourcePath);
                return true;
        }

        return true;
    }

    void AudioComponent::readProperties(gameplay::Properties & properties)
    {
        _jumpAudioSourcePath = properties.getString("jump_sound");
        _enemyDeathAudioSourcePath= properties.getString("enemy_death_sound");
        _playerDeathAudioSourcePath= properties.getString("player_death_sound");
    }
}
