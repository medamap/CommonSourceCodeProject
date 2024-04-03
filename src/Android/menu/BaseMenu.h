//
// Created by medamap on 2024/04/02.
//

#ifndef ANDROIDSTUDIO_BASEMENU_H
#define ANDROIDSTUDIO_BASEMENU_H

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

enum ItemType {
    Category,
    Property
};

class MenuNode {
public:
    int parentId;
    int nodeId;
    std::string caption;
    ItemType itemType;
    int returnValue;

    MenuNode(int parentId, int nodeId, const std::string& caption, ItemType itemType, int returnValue)
            : parentId(parentId), nodeId(nodeId), caption(caption),
              itemType(itemType), returnValue(returnValue) {}

    // ノードIDを返すメソッド
    int getNodeId() const {
        return nodeId;
    }
    // キャプションを返すメソッド
    const std::string& getCaption() const {
        return caption;
    }
    // アイテムタイプを返すメソッド
    ItemType getItemType() const {
        return itemType;
    }
    // 戻り値を返すメソッド
    int getReturnValue() const {
        return returnValue;
    }
    // 親ノードIDを返すメソッド
    int getParentId() const {
        return parentId;
    }
    // 空のメニューノードを返す
    static MenuNode emptyNode() {
        return MenuNode(-1, -1, "", Category, -1);
    }
};

class BaseMenu {
public:
    // メンバ変数の初期化を行う
    // nextNodeIdを1で初期化する
    BaseMenu() : nextNodeId(1) {}

    // ノードを追加する
    int addNode(int parentId, const std::string& caption, ItemType itemType, int returnValue) {
        int nodeId = nextNodeId++;
        nodes.emplace_back(parentId, nodeId, caption, itemType, returnValue);
        return nodeId;
    }

    // ルートノードを取得する
    std::vector<MenuNode> getRootNodes() {
        return getNodes(0);
    }

    // 親ノードIDを指定してノードを取得する
    std::vector<MenuNode> getNodes(int parentId) {
        std::vector<MenuNode> result;
        for (const auto& node : nodes) {
            if (node.parentId == parentId) {
                result.push_back(node);
            }
        }
        return result;
    }

    // ノードIDを指定してノードを最初の１つ取得する
    MenuNode getNode(int nodeId) {
        for (const auto& node : nodes) {
            if (node.nodeId == nodeId) {
                return node;
            }
        }
        return MenuNode(-1, -1, "", Category, -1);
    }

    // 指定されたノードIDの親IDを取得する
    int getParentId(int nodeId) {
        for (const auto& node : nodes) {
            if (node.nodeId == nodeId) {
                return node.parentId;
            }
        }
        return -1;
    }

    // 指定された親IDを元に階層メニュー文字列を生成する
    //         extendMenuString += std::to_string(node.getNodeId()) + ";" +
    //                            node.getCaption() + ";" +
    //                            std::to_string(node.getItemType()) + ";" +
    //                            std::to_string(node.getReturnValue()) + ";" +
    //                            std::to_string(node.getParentId()) + ",";
    std::string getExtendMenuString(int parentId) {
        std::string extendMenuString;
        for (const auto& node : nodes) {
            if (node.parentId == parentId) {
                extendMenuString += std::to_string(node.getNodeId()) + ";" +
                                   node.getCaption() + ";" +
                                   std::to_string(node.getItemType()) + ";" +
                                   std::to_string(node.getReturnValue()) + ";" +
                                   std::to_string(parentId) + ",";
            }
        }
        // 末尾の余分なカンマを削除
        if (!extendMenuString.empty()) {
            extendMenuString.pop_back();
        }
        return extendMenuString;
    }

    // ノードのメニュー文字列を生成する
    //         extendMenuString = std::to_string(node.getNodeId()) + ";" +
    //                           node.getCaption() + ";" +
    //                           std::to_string(node.getItemType()) + ";" +
    //                           std::to_string(node.getReturnValue()) + ";" +
    //                           std::to_string(node.getParentId());
    std::string getNodeString(const MenuNode& node) {
        return std::to_string(node.getNodeId()) + ";" +
               node.getCaption() + ";" +
               std::to_string(node.getItemType()) + ";" +
               std::to_string(node.getReturnValue()) + ";" +
               std::to_string(node.getParentId());
    }

    // 指定されたノードIDの階層文字列を取得する、階層文字列は親ノード(ROOT)から順に今のノードまで手繰ったものをスラッシュで区切った文字列
    std::string getHierarchyString(int nodeId) {
        std::string hierarchyString;
        int parentId = getParentId(nodeId);
        hierarchyString = getCaption(nodeId);
        while (parentId != 0) {
            hierarchyString = getCaption(parentId) + " / " + hierarchyString;
            parentId = getParentId(parentId);
        }
        return hierarchyString;
    }

    // 指定されたノードIDのキャプションを取得する
    std::string getCaption(int nodeId) {
        for (const auto& node : nodes) {
            if (node.nodeId == nodeId) {
                return node.caption;
            }
        }
        return "";
    }

    // 入力された文字列が下記フォーマットに沿っているなら合致するノードを返す
    // int ノードID, string キャプション, int ノードタイプ, int 戻り値, int 親ID の順で文字列をセミコロンで区切られている
    MenuNode getNodeFromExtendMenuString(const std::string& extendMenuString) {
        // セミコロンで区切る
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = extendMenuString.find(';');
        while (end != std::string::npos) {
            tokens.push_back(extendMenuString.substr(start, end - start));
            start = end + 1;
            end = extendMenuString.find(';', start);
        }
        tokens.push_back(extendMenuString.substr(start, end));
        // 5つの要素があるか確認
        if (tokens.size() != 5) {
            return MenuNode(-1, -1, "", Category, -1);
        }
        // それぞれの要素が正しい形式か確認
        try {
            int nodeId = std::stoi(tokens[0]);
            std::string caption = tokens[1];
            ItemType itemType = static_cast<ItemType>(std::stoi(tokens[2]));
            int returnValue = std::stoi(tokens[3]);
            int parentId = std::stoi(tokens[4]);
            return MenuNode(parentId, nodeId, caption, itemType, returnValue);
        } catch (std::invalid_argument& e) {
            return MenuNode(-1, -1, "", Category, -1);
        }
    }


    // 入力された文字列が下記フォーマットに沿っているならtrueを返す
    // int ノードID, string キャプション, int ノードタイプ, int 戻り値, int 親ID の順で文字列をセミコロンで区切られている
    static bool isValidExtendMenuString(const std::string& extendMenuString) {
        // セミコロンで区切る
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = extendMenuString.find(';');
        while (end != std::string::npos) {
            tokens.push_back(extendMenuString.substr(start, end - start));
            start = end + 1;
            end = extendMenuString.find(';', start);
        }
        tokens.push_back(extendMenuString.substr(start, end));
        // 5つの要素があるか確認
        if (tokens.size() != 5) {
            return false;
        }
        // それぞれの要素が正しい形式か確認
        try {
            std::stoi(tokens[0]);
            std::stoi(tokens[2]);
            std::stoi(tokens[3]);
            std::stoi(tokens[4]);
        } catch (std::invalid_argument& e) {
            return false;
        }
        return true;
    }


private:
    std::vector<MenuNode> nodes;
    int nextNodeId;
};

#endif //ANDROIDSTUDIO_BASEMENU_H
