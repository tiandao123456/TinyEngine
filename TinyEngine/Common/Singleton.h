#pragma once
#include <iostream>

//ÀàÄ£°å
template<typename T>
class Singleton
{
public:
	static T& GetInstance()
	{
		static T instance;
		return instance;
	}
	Singleton(T&&) = delete;
	Singleton(const T&) = delete;
	T& operator=(const T&) = delete;

protected:
	Singleton() = default;
	virtual ~Singleton() = default;
};