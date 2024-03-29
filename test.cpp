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
#include <queue>

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

template <class T, class CMP = std::less<T>, class ALLOC = std::allocator<T> >
std::set<T, CMP, ALLOC> operator * (
  const std::set<T, CMP, ALLOC> &s1, const std::set<T, CMP, ALLOC> &s2)
{
  std::set<T, CMP, ALLOC> s;
  std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
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
    int count = 0;
    //cout << temp <<endl;
    for (char sym : temp)  //read from right to left
    {
        if(isalnum(sym) || sym == '#' || sym == '_')
        {
            //cout<<"isalnum\n";
            count++;
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
    for (char i: outputList){
        res += i;
        if (isalnum(i)){
            count--;
            literpos[i] = literpos[i] + set<int>{count};
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
bool SetEqSet(std::queue<std::set<int>> check, std::set<int> cur_q ){
	while (!check.empty()) {
        std::set<int> cur_set = check.front();
        check.pop();
		if (cur_set == cur_q )
			return true;
	}
	return false;
}
Tree *bulidTree(string &polished, int &curpos){
    char b = polished.back();
    //cout<<"intree "<<b<<endl;
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
        //cout<<"inalpha "<<b<<endl;
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
    // cout << newTree->pos<<" pos"<<endl;
    // for (auto i : newTree->firstpos)
    //     cout<<i<<" ";
    // cout<<endl;
    return newTree;
}

DFA re2dfa(const std::string& regExp){
    string polished = makepoliz(regExp);
    cout<<"polished "<<polished<<endl;
    int n = 0;
    Tree *root = bulidTree(polished, n);
    //cout<<root->pos;
    set<int> firstpos = root->firstpos;
    // for (auto i : firstpos)
    //     cout<<i<<endl;
    Alphabet alphabet = Alphabet(regExp);

	for(auto it = literpos.cbegin(); it != literpos.cend(); ++it)
	{
		std::cout << it->first << " "<<std::endl;
		for (auto num : it->second){
			std::cout<<num<<" ";
		};
		std::cout<<std::endl;

	}
	std::queue<std::set<int>> queue;
	std::queue<std::set<int>> marked;
    std::map<std::set<int>, std::string> State2Name;
	std::cout << alphabet.to_string();
	DFA res = DFA(alphabet);
	for(auto it = literpos.cbegin(); it != literpos.cend(); ++it)
	{
		std::cout << it->first << " "<<std::endl;
		for (auto num : it->second){
			std::cout<<num<<" ";
		};
		std::cout<<std::endl;

	}
    std::set<int> first_state = root->firstpos;
	queue.push(first_state);
	State2Name[first_state] = std::to_string(marked.size()+queue.size()-1);
	
	res.create_state(State2Name[first_state],
					first_state.find(0) != first_state.end());
	res.set_initial(State2Name[first_state]);
	
	while (!queue.empty()) {
		std::set<int> cur_state = queue.front();
		marked.push(cur_state);
		queue.pop();
		for (char symbol : alphabet) {				
			std::set<int>  tmpstate = literpos[symbol]*cur_state;
			std::set<int> followpos_of_S={};
		
			for (auto state : tmpstate) {
				followpos_of_S.insert(followpos[state].begin(), followpos[state].end());
			}

			if (!followpos_of_S.empty()){
				if (!SetEqSet(marked, followpos_of_S) && !SetEqSet(queue, followpos_of_S) ){
					

					queue.push(followpos_of_S);
					State2Name[followpos_of_S] = std::to_string(marked.size()+queue.size()-1);
					
					res.create_state(State2Name[followpos_of_S],  
									 followpos_of_S.find(0) != followpos_of_S.end());
				}
				res.set_trans(State2Name[cur_state], symbol, State2Name[followpos_of_S]);
			}
		}
	
	}
	return res;
}


// int main(){
//     re2dfa("(ab|c)|(a|b)");

//     return 0;
// }

