#include "api.hpp"
#include <string>
#include <iostream>
#include <set>
#include <vector>
#include <string>
#include <map>

#include <iostream>
#include <vector>
#include <set>
#include <stack>

class Tree {
public:
    char elem;
    int index;
    Tree *left = nullptr;
    Tree *right = nullptr;
    bool nullable = false;
    std::set<int> firstpos, lastpos;

    Tree(char elem, int index, Tree *left, Tree *right) : elem(elem), index(index), left(left), right(right) {}

    ~Tree() {
        delete left;
        delete right;
    }

    void calculatePositions() {
        if (elem == '*') {
            nullable = true;
            firstpos = left->firstpos;
            lastpos = left->lastpos;
        } else if (elem == '|') {
            nullable = left->nullable || right->nullable;
            firstpos.insert(left->firstpos.begin(), left->firstpos.end());
            firstpos.insert(right->firstpos.begin(), right->firstpos.end());
            lastpos.insert(left->lastpos.begin(), left->lastpos.end());
            lastpos.insert(right->lastpos.begin(), right->lastpos.end());
        } else if (elem == '.') {
            nullable = left->nullable && right->nullable;
            if (left->nullable)
                firstpos.insert(right->firstpos.begin(), right->firstpos.end());
            else
                firstpos = left->firstpos;
            if (right->nullable)
                lastpos.insert(left->lastpos.begin(), left->lastpos.end());
            else
                lastpos = right->lastpos;
        } else {
            nullable = false;
            firstpos.insert(index);
            lastpos.insert(index);
        }
    }
};

std::vector<std::set<int>> followpos;
std::vector<int> char_index, operator_index, operator_priority;

Tree* buildSyntaxTree(const std::string& regex) {
    std::stack<Tree*> st;
    for (char c : regex) {
        if (c == '*' || c == '|' || c == '.') {
            Tree *right = st.top(); st.pop();
            Tree *left = st.top(); st.pop();
            Tree *node = new Tree(c, -1, left, right);
            node->calculatePositions();
            st.push(node);
        } else if (c == '(') {
            st.push(nullptr);
        } else if (c == ')') {
            Tree *node = st.top(); st.pop();
            st.pop(); // remove the null pointer
            st.push(node);
        } else if (c == 'ε') {
            Tree *node = new Tree(c, -1, nullptr, nullptr);
            node->nullable = true;
            st.push(node);
        } else {
            Tree *node = new Tree(c, char_index.size(), nullptr, nullptr);
            node->calculatePositions();
            st.push(node);
            char_index.push_back(c);
        }
    }
    return st.top();
}


DFA re2dfa(const std::string& regExp){
    std::string s1 = '(' + regExp + ")#";

    for (int i = s1.size() - 1; i > 0; i--) {
        if (s1[i] != '*' && s1[i] != '|' && s1[i - 1] != '|' && s1[i] != ')' && s1[i - 1] != '(') {
            s1.insert(i, ".");
        }
    }
    Tree* root = buildSyntaxTree(s1);
    int counter = 0;
	std::cout<<"Here"<<"\n";
	DFA res = DFA(Alphabet(regExp));
	res.create_state("Start", true);
	res.set_initial("Start");
	return res;

}