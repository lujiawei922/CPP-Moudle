#include <iostream>
#include <memory>
#include <mutex>
#include <map>
#include <functional>
using namespace std;
class Thing{
public:
    Thing(string name):name_(name){

    }

    const string& key() const { return name_; }
private:
    string name_;
};

class ObjectPool : public std::enable_shared_from_this<ObjectPool>
{
private:
    std::mutex mutex_;
    std::map<string, std::weak_ptr<Thing>> things_;

    static void weakDeleteCallback(const weak_ptr<ObjectPool>& wkObjectPool,Thing* thing){
        shared_ptr<ObjectPool> objectpool(wkObjectPool.lock());
        if(objectpool){
            objectpool->removeThing(thing);
        } else {
            cout << "ObjectPool died";
        }

        delete thing;  // sorry, I lied
    }

    void removeThing(Thing* thing){
        if(thing){
            unique_lock<mutex> lock(mutex_);
            auto it = things_.find(thing->key());
            if(it != things_.end() && it->second.expired()){
                things_.erase(thing->key());
            }
        }

    }
public:
    ObjectPool(/* args */){}
    ObjectPool(const ObjectPool&)=delete;
    ObjectPool& operator=(const ObjectPool&)=delete;
    ~ObjectPool(){}
    std::shared_ptr<Thing> get(string& key){
        std::shared_ptr<Thing> pThing;
        unique_lock<mutex> lock(mutex_);
        // 注意此处是引用
        std::weak_ptr<Thing>& wkTing = things_[key];
        // 对weak指针进行类型提升
        pThing = wkTing.lock();
        if(!pThing){ // 提升失败，表示没有该对象，需要自己创建
            // 这里自己创建对象析构时的谓词,占位符
            pThing.reset(new Thing(key),bind(&ObjectPool::weakDeleteCallback,
                                            weak_ptr<ObjectPool>(shared_from_this()),
                                            std::placeholders::_1));
            wkTing = pThing;
        }

        return pThing;
    }
};
