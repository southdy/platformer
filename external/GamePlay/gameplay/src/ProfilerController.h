#ifndef PROFILERCONTROLLER_H_
#define PROFILERCONTROLLER_H_

#include <string>
#include <vector>

namespace gameplay
{
    class ProfilerController
    {
        friend class EventRecorderInternal;
        friend class Game;
        friend class Platform;
    public:
        ~ProfilerController() {}
        class EventReader
        {
        public:
            struct Event
            {
                const char * _name;
                double _totalTime;
                double _minTime;
                double _maxTime;
                unsigned int _hits;
                unsigned int _depth;
            };
            virtual void read(const Event& event) = 0;
        };

        struct EventInternal
        {
            EventInternal();
            double _totalTime;
            double _minTime;
            double _maxTime;
            double _captureStart;
            double _firstCaptureStart;
            unsigned int _hits;
            unsigned int _depth;
            void reset();
        };
        struct EventRecorderInternal
        {
            EventRecorderInternal(const char * file, const char * function, const char * id, unsigned int line);
            void reset();
            void next();
            void start(double start);
            void finish(double duration);
            std::string _name;
            unsigned int _index;
            std::vector<EventInternal> _frames;
        };
        struct EventScopeInternal
        {
            EventScopeInternal(EventRecorderInternal * counter);
            ~EventScopeInternal();
            EventRecorderInternal * _counter;
            double _startTime;
        };

        unsigned int getFrameIndex() const;
        unsigned int getFrameHistorySize() const;
        bool isRecording() const;
        void setRecording(bool enabled);
        double getDuration(unsigned int frameIndex) const;
        void getFrameEvents(unsigned int frameIndex, EventReader * reader);
    private:
        ProfilerController();
        void update();
        unsigned int _index;
        unsigned int _maxNameSize;
        unsigned int _hits;
        const unsigned int _size;
        int _depth;
        bool _enabled;
        double _previousFrameStart;
        double _previousCaptureStart;
        double _currentCaptureStart;
        std::vector<EventRecorderInternal*> _counters;
        std::vector<double> _durations;
    };
}

#ifdef GP_USE_PROFILER
#define CONCAT_INTERNAL(A, B) A ## B
#define CONCAT(A, B) CONCAT_INTERNAL(A, B)
#define PROFILE_INTERNAL(line, id) \
    static gameplay::ProfilerController::EventRecorderInternal CONCAT(recorder,line)(__FILE__, __FUNCTION__, id, line);\
    gameplay::ProfilerController::EventScopeInternal CONCAT(scope,line)(&CONCAT(recorder,line));
#define PROFILE() PROFILE_INTERNAL(__LINE__, nullptr)
#define PROFILE_(id) PROFILE_INTERNAL(__LINE__, #id)
#else
#define PROFILE()
#define PROFILE_(id)
#endif
#endif
