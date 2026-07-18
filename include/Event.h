#ifndef __EVENT_H__
#define __EVENT_H__

#include <algorithm>
#include "Entity.h"
#include "Component.h"

// Event base class
class Event {
    public: 
    virtual ~Event() = default;
};

// Specific event types
class CollisionEvent : public Event{
    public:
        CollisionEvent(Entity* e1, Entity* e2) : entity1(e1), entity2(e2) {}
        Entity* GetEntity1() const {return entity1;}
        Entity* GetEntity2() const {return entity2;}

    private:
        Entity* entity1;
        Entity* entity2;

};

class EventListener {
    public:
        virtual ~EventListener() = default;
        virtual void OnEvent(const Event& event) = 0;
};

class EventSystem{
    public:
        void AddListener(EventListener* listener){
            listeners.push_back(listener);
        }
        //O(N)? Thats grimy
        void RemoveListener(EventListener* listener){
            auto it = std::find(listeners.begin(), listeners.end(), listener);
            if(it != listeners.end()){
                listeners.erase(it);
            }
        }
        void DispatchEvent(const Event& event){
            for(auto listener : listeners){
                listener->OnEvent(event);
            }
        }

    private:
        std::vector<EventListener*> listeners;
};

class PhysicsComponent : public Component, public EventListener{
  //Override the component initializer
  public:
  void Initialize() override{
    //Singleton?
    GetEventSystem().AddListener(this);
  }
  ~PhysicsComponent() override{
    GetEventSystem().RemoveListener(this);
    //Destroy any other resources
  }

  void OnEvent(const Event& event) override{
    if(auto collisionEvent = dynamic_cast<const CollisionEvent*>(&event)){
      //Handle collision event
    }
  }
  private:
    EventSystem& GetEventSystem(){
      static EventSystem eventSystem;
      return eventSystem;
    }
};

#endif