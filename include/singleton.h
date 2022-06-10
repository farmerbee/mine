#ifndef __SINGLETON_H
#define __SINGLETON_H

template <class T>
class Singleton{
public:
    static T* getInstance()
    {
        static T t;
        return &t;
    }

private:
    Singleton(){}
};

#endif