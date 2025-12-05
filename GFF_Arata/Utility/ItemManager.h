#pragma once
#include <vector>
#include <string>
#include "../Objects/Item.h"
#include "InputManager.h"

enum class ItemMode {
    List,   // リスト表示
    Detail  // 詳細表示
};

class ItemManager {
private:
    std::vector<Item*> items;
    bool isListOpen;  // アイテムリストが開いているか
    ItemMode mode;
    int selectedIndex;
public:
    ~ItemManager();

    void Add(Item* item);
    void Init();
    void Update(float playerX, float playerY, float deltaTime);
    void Draw(float cameraOffsetX = 0.0f) const;
    void ToggleList();  // アイテムリストの開閉
    std::vector<std::string> GetCollectedItems() const;  // 収集されたアイテム名のリストを取得
    int GetCollectedCount() const;  // 収集されたアイテム数を取得
    int GetTotalCount() const { return (int)items.size(); }  // 全アイテム数を取得

    // ★この関数を追加してください！
    // 収集されていないアイテムも含め、全てのアイテムリストへの参照を返す
    const std::vector<Item*>& GetItems() const { return items; }
};