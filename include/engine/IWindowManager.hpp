#pragma once

class IWindowManager{
public:
    virtual ~IWindowManager() = default;

    virtual void init() = 0;
    virtual void update() = 0;
    virtual void cleanup() = 0;
};