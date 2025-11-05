#pragma once
#include <vector>
#include "cocos2d.h"

enum class DropKind { NONE, COIN, UPGRADE, STAR };

struct DropSpec {
    DropKind kind;
    float    weight;    // trọng số bốc thăm
    int      minCount;
    int      maxCount;
};

class LootTable {
public:
    static const std::vector<DropSpec>& defaultEnemyTable();
    static const std::vector<DropSpec>& defaultChestTable();

    // Roll & spawn tại vị trí p (CENTER của đối tượng chết/vỡ)
    static void rollAndSpawn(cocos2d::Node* root, const cocos2d::Vec2& p,
                             const std::vector<DropSpec>& table = defaultEnemyTable());

private:
    static DropKind rollOne(const std::vector<DropSpec>& table, int& outCount);
    static void     spawn(cocos2d::Node* root, const cocos2d::Vec2& p,
                          DropKind k, int count);
};
