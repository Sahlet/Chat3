#include "stdafx.h"
#include "CppUnitTest.h"
#include <My\WinSockBaseSerialization.h>
#include <My\thread_pool.h>
#include <list>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace My;

namespace UnitTest
{		
	TEST_CLASS(UnitTest1)
	{
	public:

		/*TEST_METHOD(SocketUDP) {
			WinSocketAddress ad1("127.0.0.1", 30021), ad2("127.0.0.1", 30022);
			Socket s1(SOCK_DGRAM), s2(SOCK_DGRAM);
			s1.bind(ad1);
			s2.bind(ad2);

			std::string sending_string("Hello");
			char received_string[100];

			s1.sendto(ad2, sending_string.c_str(), sending_string.size());

			s2.recvfrom(ad1, received_string, sending_string.size());

			Assert::AreEqual(sending_string, std::string(received_string, sending_string.size()));
		}

		TEST_METHOD(SocketTCP_and_BaseSerialization) {
			WinSocketAddress ad1("127.0.0.1", 30023);
			Socket s1;
			s1.bind(ad1);
			s1.listen();
			std::string sending_string("Hello"), received_string;

			Socket s;
			s.connect(ad1);
			s << sending_string;
			
			std::thread([&]() {
					Socket s2 = s1.accept();
					s2 >> received_string;
					received_string = sending_string;
				}
			).join();

			Assert::AreEqual(sending_string, received_string);
		}*/

		TEST_METHOD(thread_pool_void_context) {
			int res;

			thread_pool<void> pool(2);

			auto future_res_1 = pool.add_task([]()->int { return 1; });
			auto future_res_2 = std::move(pool.add_task([&res, &future_res_1]()->int {
				res = 1 + future_res_1.get();
				return 0;
			}));

			Assert::AreEqual(0, future_res_2.get());
			Assert::AreEqual(2, res);
		}

		TEST_METHOD(thread_pool_int_context) {
			thread_pool< int& > pool(1);

			auto future_res = pool.add_task([]()->int { return 1; });
			pool.add_task([&future_res](int& res) { res = 1 + future_res.get(); });
			auto future_res_context = pool.add_task([](int res)->int { return res; });
			pool.add_task([]() { /*dummy function*/ });

			Assert::AreEqual(2, future_res_context.get());
		}
	};
}