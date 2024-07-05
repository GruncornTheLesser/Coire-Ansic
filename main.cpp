/* 
 TODO: ECS emplace/erase/etc funcs
 TODO: view where parameter implementation
 TODO: entity manager
 TODO: versioner
 TODO: dispatcher
 TODO: 
*/

#include "util/priority_mutex.h"
#include <thread>
#include <iostream>

std::mutex print_mtx;
void print(std::string str) {
	std::lock_guard lk(print_mtx);
	std::cout << str << std::endl;
}

void work(util::priority_mutex& mtx, util::priority p, bool shared, std::string thread_name) 
{
	if (shared) mtx.shared_lock(p);
	else mtx.exclusive_lock(p); 

	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	
	std::string str = thread_name;
	switch (p) 
	{
		case util::priority::HIGH: str += " - high";
		case util::priority::MEDIUM: str += " - medium";
		case util::priority::LOW: str += " - low";
	}
	str += shared ? " - shared" : " - exclusive";
	print(str);

	mtx.exclusive_unlock();
}

std::chrono::milliseconds delay{ 400 };

int main() 
{
	util::priority_mutex mtx;

	
	mtx.exclusive_lock(util::priority::HIGH);
	
	std::thread t1{ work, std::ref(mtx), util::priority::HIGH, false, "t1" };
	std::thread t2{ work, std::ref(mtx), util::priority::MEDIUM, false, "t2" };
	std::thread t3{ work, std::ref(mtx), util::priority::LOW, true, "t3" };
	std::thread t4{ work, std::ref(mtx), util::priority::HIGH, false, "t4" };
	std::thread t5{ work, std::ref(mtx), util::priority::MEDIUM, true, "t5" };
	std::thread t6{ work, std::ref(mtx), util::priority::LOW, true, "t6" };
	std::thread t7{ work, std::ref(mtx), util::priority::HIGH, false, "t7" };
	mtx.exclusive_unlock();
	

	
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	t6.join();
	t7.join();





//	res::registry reg;
//	std::cout << util::type_name<res::lock<res_B>>() << std::endl;
//	std::cout << util::type_name<res::traits::get_lockset_t<res::lock<res_B>>>() << std::endl;
//	std::cout << util::type_name<res::traits::get_lockset_t<res_B>>() << std::endl;
//	std::cout << res::traits::is_accessible_v<res_B, res::lock<res_B>> << std::endl;
//	std::thread p1(process<res_B>, &reg);
	//std::thread p2(process<res_D>, std::ref(reg));
	
	

}

