#include "loki.pch.h"



int main() {
	std::cout << "Hello world" << std::endl;

	loki::movegen::move_stack<16> m_stack;

	for (loki::move_t i = 0; i < 16; i++) {
		m_stack.insert(i, loki::movegen::lost_move_info{});
		std::cout << "Inserted " << i << std::endl;
	}

	for (loki::move_t i = 0; i < 16; i++) {
		std::cout << "Popped " << m_stack.pop().first << std::endl;
	}
}