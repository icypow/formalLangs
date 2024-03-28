#include "api.hpp"
#include <string>
#include <iostream>
#include <set>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <vector>
#include <set>
#include <stack>
#include <algorithm>


using namespace std;
int Priority(const char &c);
bool isOperator(const char &c);
string prettify(const string reg);
map<int, set<int>> followpos;
map<char, set<int>> literpos;


template <class T, class CMP = std::less<T>, class ALLOC = std::allocator<T> >
std::set<T, CMP, ALLOC> operator + (
  const std::set<T, CMP, ALLOC> &s1, const std::set<T, CMP, ALLOC> &s2)
{
  std::set<T, CMP, ALLOC> s;
  std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
    std::inserter(s, s.begin()));
  return s;
}
struct Tree{
    bool nullable;
    Tree *left = nullptr, *right = nullptr;
    set<int> firstpos = {}, lastpos = {};
    int pos;
};


string makepoliz(const string regExp){
    string res;
    vector<char> outputList;//output vector
    stack<char> s;//main stack
    string temp = prettify(regExp);
    cout << temp <<endl;
    for (char sym : temp)  //read from right to left
    {
        if(isalnum(sym) || sym == '#' || sym == '_')
        {
            //cout<<"isalnum\n";
            outputList.push_back(sym);
        }
        if(sym == '(')
        {
            s.push(sym);
        }
        if(sym == ')')
        {
            while(!s.empty() && s.top()!= '(')
            {
                outputList.push_back(s.top());
                s.pop();
            }
            s.pop();
        }
        if(isOperator(sym) == true)
        {
            while(!s.empty() && Priority(s.top()) >= Priority(sym))
            {
                outputList.push_back(s.top());
                s.pop();
            }
            s.push(sym);
        }
    }
    while(!s.empty())
    {
        outputList.push_back(s.top());
        s.pop();
    }
    int counter = 0;
    for (char i: outputList){
        res += i;
        if (isalnum(i)){
            counter++;
            literpos['i'] = literpos['i'] + set<int>{counter};
        }
    }
    //cout<<res<<'\n';
    return res;
}

string prettify(const string regExp){
    string res;
    res.push_back(regExp[0]);
    for ( int i = 1 ; i < regExp.size(); ++i){
        if ((regExp[i-1] == '*' || regExp[i-1] == ')' || isalnum(regExp[i-1]))&&(regExp[i] == '(' || isalnum(regExp[i]))&&(regExp[i] != '*'))
            res.push_back('.');
        if ((regExp[i-1] == '(' || regExp[i-1] == '|')&&((regExp[i] == ')' || regExp[i] == '|' )))
            res.push_back('_');
        //cout<<regExp[i]<<endl;
        res.push_back(regExp[i]);
    }
	res = '(' + res + ").#";
    //cout<<res<<'\n';
    return res;
}

int Priority(const char &c)
{
    if(c == '*') return 3;
    if(c == '.') return 2;
    if(c== '|') return 1;
    else return 0;
}
bool isOperator(const char &c)
{
    return (c == '*' || c == '.' || c =='|');
}

Tree *bulidTree(string &polished, int &curpos){
    char b = polished.back();
    cout<<"intree "<<b<<endl;
    polished.pop_back();
    Tree *newTree = new Tree;
    if (b == '.' || b == '|'){
        newTree->right = bulidTree(polished, curpos);
        newTree->left = bulidTree(polished, curpos);
        if (b == '.'){
            newTree->nullable = newTree->left->nullable && newTree->right->nullable;
            for (auto i : newTree->left->lastpos)
                followpos[i] = followpos[i] + newTree->right->firstpos;
        }
        else newTree->nullable = newTree->left->nullable || newTree->right->nullable;
        if (newTree->left->nullable || b == '|'){
            newTree->firstpos = newTree->left->firstpos + newTree->right->firstpos;
        }
        newTree->firstpos = newTree->firstpos + newTree->left->firstpos;
        if (newTree->right->nullable || b == '|'){
            newTree->lastpos = newTree->left->lastpos + newTree->right->lastpos;
        }
        newTree->lastpos = newTree->lastpos + newTree->right->lastpos;
    }
    else if (b == '*'){
        newTree->left = bulidTree(polished, curpos);
        newTree->nullable = true;
        newTree->firstpos = newTree->left->firstpos;
        newTree->lastpos = newTree->left->lastpos;
        for (auto i : newTree->left->lastpos)
            followpos[i] = followpos[i] + newTree->left->firstpos;
    }
    else {
        cout<<"inalpha "<<b<<endl;
        newTree->pos = curpos;
        if (b == '_') {
            newTree->nullable = true;
        }
        else {
            newTree->nullable = false;
            newTree->firstpos = newTree->firstpos + set<int>{curpos};
            newTree->lastpos = newTree->lastpos + set<int>{curpos};
        }
        curpos++;
    }
    cout << newTree->pos<<" pos"<<endl;
    for (auto i : newTree->firstpos)
        cout<<i<<" ";
    cout<<endl;
    return newTree;
}

void re2dfa(const std::string& regExp){
    string polished = makepoliz(regExp);
    cout<<"polished "<<polished<<endl;
    int n = 0;
    Tree *root = bulidTree(polished, n);
    //cout<<root->pos;
    set<int> firstpos = root->firstpos;
    for (auto i : firstpos)
        cout<<i<<endl;

    // Alphabet alphabet = Alphabet(regExp);
	// DFA res = DFA(alphabet);
    // std::vector<std::set<int>> states;
    // states.push_back(root->firstpos);

    // res.create_state(std::to_string(0), root->nullable);
    // res.set_initial(std::to_string(0));
    // for (int index = 0; index < states.size(); index++) {
    //     for (auto &alpha_from_alphabet: alphabet) {
    //         std::set<int> new_state;
    //         for (auto &index_alpha_in_cur_state: states[index]) {
    //             if (polished[literpos[index_alpha_in_cur_state]] == alpha_from_alphabet) {
    //                 new_state.insert(followpos[index_alpha_in_cur_state].begin(),
    //                                  followpos[index_alpha_in_cur_state].end());
    //             }
    //         }

    //         if (!new_state.empty()) {
    //             int index_to = 0;
    //             for (; index_to < states.size(); index_to++) {
    //                 if (states[index_to] == new_state) {
    //                     break;
    //                 }
    //             } //existing state
    //             if (index_to == states.size()) {
    //                 //new state
    //                 index_to = states.size();
    //                 states.push_back(new_state);
    //                 res.create_state(std::to_string(index_to), new_state.count(literpos.size() - 1) == 1);
    //             }
    //             res.set_trans(std::to_string(index), alpha_from_alphabet, std::to_string(index_to));
    //         }
    //     }
    // }


    //return res;
}


int main(){
    re2dfa("|||||a|bc");

    return 0;
}

