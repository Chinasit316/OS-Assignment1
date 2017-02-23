#include<thread>
#include<chrono>
#include<cinttypes>
#include<mutex>
#include<iostream>
using namespace std::chrono;
typedef std::chrono::steady_clock Clock;


class MyClass
{
private:
	//#define SIZE 1000
	long int head = 0;
	long int tail = 0;
	int size = 1000;
	int *buffer = new int[size];
	std::mutex lock;
	std::condition_variable empty;
	std::condition_variable full;
public:
	long int count = 0;
	int Cfull = 0;
	int Cempty = 0;
	long int req = 0;

	MyClass::MyClass(int a,long int b) {
	   size = a;
	   req = b;
	   int *buff = new int[size];
	   *buffer = *buff;
	}

	void add_item(long int val) {
		buffer[tail%size] = val;
		tail++;
		//printf("push %-4d", tail - head);
	}

	long int remove_item() {
		long int ret = NULL;
		ret = buffer[head%size];
		head++;
		//printf("pop  %-4d", tail - head);
		return ret;
	}

	void append(long int val) {
		while (true)
		{
			lock.lock();
			if (req <= 0) {
				lock.unlock();
				break;
			}
			lock.unlock();
			std::unique_lock<std::mutex> lk(lock);
			while (tail >= head + size) {
				Cfull++;
				full.wait_for(lk,std::chrono::seconds(3));
			}
			req--;
			add_item(val);
			empty.notify_all();
			lk.unlock();
		}
	}

	void remove() {
		while (true)
		{
			lock.lock();
			if (tail==head&&req <= 0) {
				lock.unlock();
				break;
			}
			lock.unlock();
			std::unique_lock<std::mutex> lk(lock);
			while (tail - head <= 0 ) {
				Cempty++;
				empty.wait_for(lk,std::chrono::seconds(3));
			}
			/*int x = NULL;
			x = remove_item();*/
			remove_item();
			full.notify_all();
			lk.unlock();
			//return x;
		}
	}
};



int main(int argc, char *argv[])
{
	long int a = 0;
	long int b = 0;
	long int c = 0;
	long int d=0;
	std::cout << "buff ";
	std::cin >> a >> b >> c >> d;

	MyClass Cbuff(c,d);
	//std::thread add(Cbuff.append);
	Clock::time_point t1 = Clock::now();
	std::thread producer(
		[&]() {
		for (int i = 0; i<a; i++) {
			Cbuff.append(i);
		}
	}  // End of lambda expression
	);
	std::thread consumer(
		[&]() {
		for (int i = 0; i<b; i++)
		{
			Cbuff.remove();
		}
	}  // End of lambda expression
	);
	
	producer.join();
	consumer.join();

	Clock::time_point t2 = Clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

	std::cout << "\nProducers " << a << ", Consumers " << b << "\n";
	std::cout << "Buffer size " << c << "\n";
	std::cout << "Requests " << d << "\n\n";
	std::cout << "Elapsed Time: " << time_span.count() << " seconds.\n";
	double s = d - (Cbuff.Cfull + Cbuff.Cempty);
	std::cout << "Successfull " <<s<< " requests" << "("<< s/d*100 <<"%)\n";
	std::cout << "Buffer full " << Cbuff.Cfull << " Time\n";
	std::cout << "Buffer empty " << Cbuff.Cempty << " Time\n";
	std::cout << "Throughput: " << d/ time_span.count() << " requests/s\n";
	return 0;
}
