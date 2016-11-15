#include "query_processing.h"

using namespace My;
using namespace std;

const unsigned int max_conncectins_count = 0b11111111111111111;
const unsigned int max_user_count = 0b111111111111111111111;

std::unique_ptr < user, std::default_delete < user[] > > users_guard;
user* users;

string BASEhost, BASEuser, BASEpassword;
unsigned int BASEport;

int main(int argc, char *argv[]) {
	LOG_FILES::out = stdout;
	LOG_FILES::warn = stdout;
	LOG_FILES::err = stderr;

	struct system_pause {
		~system_pause() {
			std::system("pause");
		}
	} pause_maker;
	try {
#pragma region Connecting to DATABASE
		{
			string s_port;
			if (argc != 5) {
				cout << "Enter host:\n"; cin >> BASEhost;
				cout << "Enter port:\n"; cin >> s_port;
				cout << "Enter login:\n"; cin >> BASEuser;
				cout << "Enter password:\n"; cin >> BASEpassword;
			}
			else { BASEhost = argv[1]; s_port = argv[2]; BASEuser = argv[3]; BASEpassword = argv[4]; }


			int port = atoi(s_port.c_str());
			if (port < 0 || (port == 0 && s_port != "0")) throw std::exception("bad port");
			BASEport = port;

			mysqlWrap test_connection(BASEhost.c_str(), BASEport, BASEuser.c_str(), BASEpassword.c_str());

			cout << "connected.\n";
		}
		
#pragma endregion
		users_guard.reset(new user[max_user_count]);
		users = users_guard.get();
		try {
			struct joiner_guard {
				My::interruptible_thread& thread;
				joiner_guard(My::interruptible_thread& thread) : thread(thread) {}
				~joiner_guard() {
					thread.interrupt();
					thread.join();
				}
			};
			My::interruptible_thread cleaner([]() {
				try {
					while (true) {
						for (int i = 0; i < max_user_count; i++) {
							My::interruption_point();
							users[i].clear_old_contexts();
							if (!(i % 1000)) std::this_thread::yield();
						}
						std::this_thread::sleep_for(std::chrono::seconds(5));
					}
				} catch (const interruption&) {}
			});
			bool quit = false;
			My::interruptible_thread processor_of_input([&quit]() {
				try {
					while (true) {
						My::interruption_point();
						std::string s;
						std::cin >> s;
						if (s == "quit") {
							quit = true;
							return;
						}
					}
				} catch (const interruption&) { quit = true; }
			});
			joiner_guard cleaner_guard(cleaner);
			joiner_guard processor_of_input_guard(processor_of_input);
			
			WinSocket sock;
			WSAPOLLFD descriptor = { sock.get() , POLLIN , 0 };
			sock.bind(WinSocketAddress("", 30000));
			sock.listen(max_conncectins_count);
			u_long ioctlsocket_arg = 1;
			if (::ioctlsocket(sock.get(), FIONBIO, &ioctlsocket_arg) == SOCKET_ERROR) throw My::WinSocketException("main: error in ::ioctlsocket for sock.");
			My::thread_pool< mysqlWrap& > pool(3, max_conncectins_count);

			//pool.add_task([] { test("l1"); });
			pool.add_task([] { test("l1", "p"); });

			long long accepted = 0;
			while (!quit) {
				int res = ::WSAPoll(&descriptor, 1, 1000);
				if (res == SOCKET_ERROR) throw My::WinSocketException("main: error in ::WSAPoll.");
				if (!bool(res)) continue;

				std::unique_ptr< WinSocket > socket_guard;
				try {
					auto socket = sock.accept();
					
					u_long ioctlsocket_arg = 0;
					if (::ioctlsocket(socket.get(), FIONBIO, &ioctlsocket_arg) == SOCKET_ERROR) throw My::WinSocketException("main: error in ::ioctlsocket for socket.");

					socket_guard.reset(new WinSocket(std::move(socket)));
				} catch (const My::WinSocketException& ex) {
					auto errorCode = ex.get_errorCode();
					if (errorCode == WSAEWOULDBLOCK || errorCode == WSAECONNABORTED) continue;
					else throw;
				}
				cout << "accepted " << ++accepted << endl;
				WinSocket socket = std::move(*socket_guard);
				pool.add_task([socket](mysqlWrap& connection)
					{
						try {
							if (!connection) {
								connection = mysqlWrap(BASEhost.c_str(), BASEport, BASEuser.c_str(), BASEpassword.c_str());
							}
							query_processor(socket, connection);
						} catch (const WinSocketException& ex) {
							cerr << ex;
						} catch (const mysqlException& ex) {
							if  (
									ex.get_errorCode() == ER_ABORTING_CONNECTION || // Aborted connection to database
									ex.get_errorCode() == ER_NEW_ABORTING_CONNECTION || // Aborted connection to database
									ex.get_errorCode() == CR_SERVER_LOST || // Lost connection to MySQL server during query
									ex.get_errorCode() == CR_INVALID_CONN_HANDLE || // Invalid connection handle
									ex.get_errorCode() == CR_SERVER_LOST_EXTENDED // Lost connection to MySQL server
								) {
								cerr << ex;
								connection = mysqlWrap(BASEhost.c_str(), BASEport, BASEuser.c_str(), BASEpassword.c_str());
							} else throw;
						}
					}
				);
			}
		} catch (...) {
			cout << "server crashed.\n";
			throw;
		}
	} catch (const Exception& ex) {
		cerr << ex;
		return ex.get_errorCode();
	} catch (const std::exception& ex) {
		cerr << "ERROR: " << ex.what() << endl;
		return 1;
	}
	return 0;
}

//int main() { return 0; }