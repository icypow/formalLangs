#include "api.hpp"
#include <string>
#include <iostream>
#include <set>
#include <vector>
#include <string>
#include <map>

template <typename T>
std::ostream & operator << (
                std::ostream & os,
                const std::vector<T> & vec)
{
    for(auto elem : vec)
    {
        os<<elem<< " ";
    }
    return os;
}


class Tree {
public:
    char symbol;
    bool nullable;
    std::vector<int> firstpos, lastpos;
    Tree* left;
    Tree* right;

    Tree(char symbol) : symbol(symbol), left(nullptr), right(nullptr) {
        nullable = false;
    }
};

std::map<int, std::vector<int>> followpos;


Tree* buildSyntaxTree(const std::string& regex) {
    std::vector<Tree*> stack;
    for (char ch : regex) {
        if (ch == '(') continue; // пропускаем открывающую скобку
        Tree* node = new Tree(ch);
        if (ch == '*' || ch == '.' || ch == '|') {
            node->right = stack.back(); stack.pop_back();
            if (ch == '.' || ch == '|') {
                node->left = stack.back(); stack.pop_back();
            }
        }
        stack.push_back(node);
        if (!stack.empty() && stack.back()->symbol == ')') { // если встретили закрывающую скобку
            stack.pop_back(); // удаляем ее из стека
            Tree* subTree = stack.back(); stack.pop_back(); // извлекаем поддерево
            stack.back()->right = subTree; // подключаем его к правому потомку вершины, которая была перед скобками
        }
    }
    return stack.back();
}


void calculatePositions(Tree* node, int& counter) {
    if (!node) return;
    calculatePositions(node->left, counter);
    calculatePositions(node->right, counter);

    if (!node->left && !node->right) {
        node->firstpos.push_back(++counter);
        node->lastpos.push_back(counter);
    } else if (node->symbol == '.') {
        node->nullable = node->left->nullable && node->right->nullable;
        node->firstpos = node->left->nullable ? 
            std::vector<int>(node->left->firstpos.begin(), node->left->firstpos.end()) : 
            std::vector<int>(node->right->firstpos.begin(), node->right->firstpos.end());
        node->lastpos = node->right->nullable ? 
            std::vector<int>(node->left->lastpos.begin(), node->left->lastpos.end()) : 
            std::vector<int>(node->right->lastpos.begin(), node->right->lastpos.end());
    } else if (node->symbol == '|') {
        node->nullable = node->left->nullable || node->right->nullable;
        node->firstpos.insert(node->firstpos.end(), node->left->firstpos.begin(), node->left->firstpos.end());
        node->firstpos.insert(node->firstpos.end(), node->right->firstpos.begin(), node->right->firstpos.end());
        node->lastpos.insert(node->lastpos.end(), node->left->lastpos.begin(), node->left->lastpos.end());
        node->lastpos.insert(node->lastpos.end(), node->right->lastpos.begin(), node->right->lastpos.end());
    } else if (node->symbol == '*') {
        node->nullable = true;
        node->firstpos = node->right->firstpos;
        node->lastpos = node->right->lastpos;
    }

    if (node->symbol == '.' || node->symbol == '*') {
        for (int i : node->left->lastpos) {
            followpos[i].insert(followpos[i].end(), node->right->firstpos.begin(), node->right->firstpos.end());
        }
    }
}


void printTree(Tree* node, int level = 0) {
    if (!node) return;
    printTree(node->right, level + 1);
    for (int i = 0; i < level; i++) std::cout << "  ";
    std::cout << node->symbol << "\n";
    printTree(node->left, level + 1);
}


DFA re2dfa(const std::string& regExp){

    Tree* root = buildSyntaxTree(regExp);
    int counter = 0;
    calculatePositions(root, counter);
	std::cout<<"Here"<<"\n";

    printTree(root);
	for(auto it = followpos.cbegin(); it != followpos.cend(); ++it)
		{
			std::cout << it->first << " " << it->second << "\n";
		}
	DFA res = DFA(Alphabet(regExp));
	res.create_state("Start", true);
	res.set_initial("Start");
	return res;

}