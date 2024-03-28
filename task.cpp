#include "api.hpp"
#include <string>
#include <algorithm>
#include <iostream>
#include <stack>
#include <map>
#include <set>
#include <queue>


class node {
  public:
    char symbol;
    node *left = nullptr;
    node *right = nullptr;
    bool nullable =  false;
	int pos_num = 0; 
    std::set<int> firstpos = {}; 
    std::set<int> lastpos = {}; 
};

/**
 * \brief Строит расширенное регулярное выражение.                     
 * \param rv: корректное регулярное выражение.
 * \param alphabet: алфавит выражения.
 * \return Расширенное ругулярное выражение с '&', '_', '#'.
 */
std::string complete_rv(const std::string &rv, Alphabet &alphabet){

  
  std::string full_rv;
  full_rv.insert(0, 1, '(');
  full_rv.push_back(rv[0]);

  for ( int i = 1 ; i < rv.size(); ++i){

	if (rv[i-1] == '*' || rv[i-1] == ')' || alphabet.has_char(rv[i-1]) )
		if (rv[i] == '(' || alphabet.has_char(rv[i]))
			full_rv.push_back('&');
    if (rv[i-1] == '(' || rv[i-1] == '|')
        if (rv[i] == ')' || rv[i] == '|' )
        	full_rv.push_back('_');

	full_rv.push_back(rv[i]);

  }
	full_rv.push_back(')');
	full_rv.push_back('&');
	full_rv.push_back('#');

  return full_rv;
}


/**
 * \brief Строит польскую запись по выражению.                     
 * \param full_rv: Расширенное ругулярное выражение.
 * \param alphabet: алфавит выражения.
 * \return Польская запись расширенного РВ.
 */
std::string full_rv2polish(const std::string &full_rv, Alphabet alphabet){

	std::string output;
	std::stack<char> operands_stack;
	std::map<char, int> priority = {{'(', 1},{'|', 2},{'&', 3},{'*', 4}};
	alphabet.insert('#'); 
	alphabet.insert('_'); 
	for (char sym : full_rv) {

		if ( alphabet.has_char(sym) ) 
			output.push_back(sym);

		else
			switch (sym) {
				case ')':
					while (operands_stack.top() != '(') {
						output.push_back(operands_stack.top());
						operands_stack.pop();
					}
					operands_stack.pop();
					break;

				case '(':
					operands_stack.push(sym);
					break;

				default:
					while (!operands_stack.empty() && (priority[sym] <= priority[operands_stack.top()])) {
						output.push_back(operands_stack.top());
						operands_stack.pop();
					}
					operands_stack.push(sym);
					break;
			}
			
	}
	while (!operands_stack.empty()) {
		output.push_back(operands_stack.top());
		operands_stack.pop();
	}
	
	return output;
}


/**
 * \brief Строит синтаксическое дерево по польской записи РВ, заполняет таблицу followpos                     
 * \param s: Польская запись ругулярного выражения.
 * \param num: Текущая позиция
 * \param followpos: Пустая таблица pos <=> followpos
 * \return Ссылка на корень дерева.
 */
node * polish2tree(std::string &str, int &num,
			   std::map<int, std::set<int>> &followpos,
			   std::map<char, std::set<int>> &charpos) {
					
	if (!str.size()) return nullptr;
	
	char cur_char = str.back();
	str.pop_back();
	node * elem = new node;
	elem->symbol = cur_char;
    std::cout<<"curchar"<<cur_char<<std::endl;
	switch (cur_char){

		case '|':
			elem->right = polish2tree(str, num, followpos, charpos);
			elem->left = polish2tree(str, num, followpos, charpos);
			elem->nullable = elem->left->nullable || elem->right->nullable;
			elem->firstpos.insert(elem->left->firstpos.begin(), elem->left->firstpos.end());
			elem->firstpos.insert(elem->right->firstpos.begin(), elem->right->firstpos.end());
			elem->lastpos.insert(elem->left->lastpos.begin(), elem->left->lastpos.end());
			elem->lastpos.insert(elem->right->lastpos.begin(), elem->right->lastpos.end());
			break;

		case '&':
			elem->right = polish2tree(str, num,followpos, charpos);
			elem->left = polish2tree(str, num,followpos, charpos);
			elem->nullable = elem->left->nullable && elem->right->nullable;
			if (elem->left->nullable) 
				elem->firstpos.insert(elem->right->firstpos.begin(), elem->right->firstpos.end());
			if (elem->right->nullable) 
				elem->lastpos.insert(elem->left->lastpos.begin(), elem->left->lastpos.end());

			elem->firstpos.insert(elem->left->firstpos.begin(), elem->left->firstpos.end());
			elem->lastpos.insert(elem->right->lastpos.begin(), elem->right->lastpos.end());

			for (auto i : elem->left->lastpos) 
				followpos[i].insert(elem->right->firstpos.begin(), elem->right->firstpos.end());
			break;

		case '*':
			elem->left = polish2tree(str, num,followpos,charpos);
			elem->nullable = true;
			elem->firstpos.insert(elem->left->firstpos.begin(), elem->left->firstpos.end());
			elem->lastpos.insert(elem->left->lastpos.begin(), elem->left->lastpos.end());
			for (auto i : elem->left->lastpos) 
				followpos[i].insert(elem->left->firstpos.begin(), elem->left->firstpos.end());
			break;

		default:
			elem->pos_num = num;
			charpos[cur_char].insert(num);
			num += 1;
			if (cur_char == '_') {
				elem->nullable = true;
			} else {
				elem->nullable = false;
				elem->firstpos.insert(elem->pos_num);
				elem->lastpos.insert(elem->pos_num);
			}
	}
	return elem;
}
		
/**
 * \brief Проверяет наличие состояния в множестве состояний                     
 * \param check: Множество состояний.
 * \param cur_q: Текуще состояние.
 * \return true/false
 */		
bool SetEqSet(std::queue<std::set<int>> check, std::set<int> cur_q ){
	while (!check.empty()) {
        std::set<int> cur_set = check.front();
        check.pop();
		if (cur_set == cur_q )
			return true;
	}
	return false;
}

    

DFA re2dfa(const std::string &s) {
	
	if (s.empty()){
		DFA res = DFA(Alphabet("#"));
		res.create_state("null", true);
		res.set_initial("null");
		return res;
	}
	
	Alphabet alphabet = Alphabet(s);
	
	

	std::map<int, std::set<int>> followpos; // таблица pos <=> followpos
	std::map<char, std::set<int>> charpos; //   символ <=> позиции в РВ
	std::map<std::set<int>, std::string> State2Name; // состояние <=> имя

	followpos[0] = {};
	int num = 0;
	node *rv_tree;
	std::string tmp;
	tmp = complete_rv(s, alphabet);
	

	tmp = full_rv2polish(tmp,alphabet);
	
	rv_tree = polish2tree(tmp, num, followpos, charpos);
    std::cout<<tmp<<std::endl;
	for (auto i : rv_tree->firstpos) std::cout<<i<<std::endl;
	std::set<int> first_state = rv_tree->firstpos;
	
	std::queue<std::set<int>> queue;
	std::queue<std::set<int>> marked;
	std::cout << alphabet.to_string();
	DFA res = DFA(alphabet);

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
			std::set<int> S={};
			set_intersection(charpos[symbol].begin(), charpos[symbol].end(), 
								cur_state.begin(), cur_state.end(),
								std::inserter(S, S.begin()));
			
			std::set<int> followpos_of_S={};
		
			for (auto state : S) {
				
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