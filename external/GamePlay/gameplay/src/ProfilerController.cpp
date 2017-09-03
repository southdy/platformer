#include "Base.h"
#include "ProfilerController.h"
#include "Game.h"

namespace gameplay
{
    ProfilerController::ProfilerController() :
        _index(0),
        _depth(-1),
        _size(60 * 5),
        _enabled(true),
        _previousFrameStart(0),
        _currentCaptureStart(0),
        _previousCaptureStart(0)
    {
        _durations.resize(_size, 0);
    }

    void ProfilerController::update()
    {
#ifdef GP_USE_PROFILER
        const double currentFrameStart = Game::getAbsoluteTime();
        if(_enabled)
        {
            _durations[_index] = currentFrameStart - _previousFrameStart;
            ++_index;
            _depth = -1;
            _hits = 0;
            const bool reset = _index >= _size;
            if(reset)
            {
                _index = 0;
                _previousCaptureStart = _currentCaptureStart;
                _currentCaptureStart = currentFrameStart;
            }
            for(EventRecorderInternal * counter : _counters)
            {
                if(reset)
                {
                    counter->reset();
                }
                else
                {
                    counter->next();
                }
            }
        }
        _previousFrameStart = currentFrameStart;
#endif
    }

    unsigned int ProfilerController::getFrameIndex() const
    {
        return _index;
    }

    unsigned int ProfilerController::getFrameHistorySize() const
    {
        return _size;
    }

    bool ProfilerController::isRecording() const
    {
        return _enabled;
    }

    void ProfilerController::setRecording(bool enabled)
    {
        _enabled = enabled;
    }

    double ProfilerController::getDuration(unsigned int frameIndex) const
    {
        return _durations[frameIndex];
    }

    void ProfilerController::getFrameEvents(unsigned int frameIndex, EventReader * reader)
    {
        GP_ASSERT(frameIndex >= 0 && frameIndex < _size);
        std::sort(_counters.begin(), _counters.end(), [&frameIndex](EventRecorderInternal* a, EventRecorderInternal* b)
        {
            return b->_frames[frameIndex]._firstCaptureStart > a->_frames[frameIndex]._firstCaptureStart;
        });
        for(int i = 0; i < _counters.size(); ++i)
        {
            const EventRecorderInternal* counter = _counters[i];
            const EventInternal& frame = counter->_frames[frameIndex];
            if(frame._hits > 0  && (frame._captureStart == _currentCaptureStart || frame._captureStart == _previousCaptureStart))
            {
                static EventReader::Event event;
                event._name = counter->_name.c_str();
                event._totalTime = frame._totalTime;
                event._minTime = frame._minTime;
                event._maxTime = frame._maxTime;
                event._hits = frame._hits;
                event._depth = frame._depth;
                reader->read(event);
            }
        }
    }

    void ProfilerController::EventRecorderInternal::start(double start)
    {
        ProfilerController * profiler = Game::getInstance()->getProfilerController();
        if(profiler && profiler->_enabled)
        {
            EventInternal & frame = _frames[_index];
            if (frame._firstCaptureStart == 0.0)
            {
                frame._firstCaptureStart = start;
            }
            ++profiler->_hits;
            ++profiler->_depth;
        }
    }

    void ProfilerController::EventRecorderInternal::finish(double duration)
    {
        ProfilerController * profiler = Game::getInstance()->getProfilerController();
        if(profiler && profiler->_enabled)
        {
            EventInternal & frame = _frames[_index];
            ++frame._hits;
            frame._totalTime += duration;
            frame._minTime = min(frame._minTime, duration);
            frame._maxTime = max(frame._maxTime, duration);
            frame._captureStart = profiler->_currentCaptureStart;
            frame._depth = profiler->_depth;
            --profiler->_depth;
        }
    }

    ProfilerController::EventRecorderInternal::EventRecorderInternal(const char * file, const char * function, const char * id, unsigned int line)
    {
        std::string fileShort(file);
        auto end = fileShort.find_last_of('.');
        auto start = fileShort.find_last_of('/');
        if(start == std::string::npos)
        {
            start = fileShort.find_last_of('\\');
        }
        start += 1;
        _name = fileShort.substr(start, end - start) + ':' + std::string(function);
        if(id)
        {
            _name += ':' + std::string(id);
        }
        _name += ':' + toString(line);
        ProfilerController * profiler = Game::getInstance()->getProfilerController();
        _frames.resize(profiler->_size, EventInternal());
        profiler->_counters.push_back(this);
        profiler->_maxNameSize = std::max(profiler->_maxNameSize, static_cast<unsigned int>(_name.size()));
        _index = profiler->_index;
    }

    void ProfilerController::EventRecorderInternal::next()
    {
        ++_index;
        _frames[_index].reset();
    }

    void ProfilerController::EventRecorderInternal::reset()
    {
        _index = 0;
        _frames[_index].reset();
    }

    ProfilerController::EventInternal::EventInternal()
    {
        reset();
    }

    void ProfilerController::EventInternal::reset()
    {
        _hits = 0;
        _maxTime = 0;
        _minTime = std::numeric_limits<double>::max();
        _totalTime = 0;
        _firstCaptureStart = 0;
    }

    ProfilerController::EventScopeInternal::EventScopeInternal(ProfilerController::EventRecorderInternal * counter) :
        _counter(counter)
    {
        _startTime = Game::getAbsoluteTime();
        _counter->start(_startTime);
    }

    ProfilerController::EventScopeInternal::~EventScopeInternal()
    {
        _counter->finish(Game::getAbsoluteTime() - _startTime);
    }
}
