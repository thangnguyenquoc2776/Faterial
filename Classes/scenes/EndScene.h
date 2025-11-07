#pragma once
#include "cocos2d.h"

class EndScene : public cocos2d::Scene {
public:
    static EndScene* createWithText(const std::string& msg);
    virtual bool initWithText(const std::string& msg);

private:
    void _buildUI(const std::string& msg);
};
