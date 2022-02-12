#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lck(_mutex);
    _cond.wait(lck, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

        // remove last vector element from queue
    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true) 
    {
        auto phase = _phaseQueue.receive();
        if (phase == TrafficLightPhase::green) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));   
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{   
    
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    //double cycleDuration = 2000; // duration of a single simulation cycle in ms
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;

    lastUpdate = std::chrono::system_clock::now(); 
    int random;
        
    std::uniform_int_distribution<int> uni(4000,6000);
     while(true) {
        //std::random_device rd;
        //std::mt19937 gen(rd());
        //random = uni(gen);
        random = rand() % 3000+4000;

        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >=   random) { //cycleDuration
        std::cout << "Random: " << random << std::endl;
            //std::this_thread::sleep_for(std::chrono::seconds(random_integer));
            if (this->getCurrentPhase() == TrafficLightPhase::green) {
                this->_currentPhase = TrafficLightPhase::red;
                //std::cout << "Intersection #" << _id << ": Traffic Light switched to red!" << std::endl;
            }
            else {
                this->_currentPhase = TrafficLightPhase::green;
                //std::cout << "Intersection #" << _id << ": Traffic Light switched to green!" << std::endl;
            }
            _phaseQueue.send(std::move(_currentPhase));
            lastUpdate = std::chrono::system_clock::now();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

