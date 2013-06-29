#include <iostream>
#include <string>
#include <thread>

#include <unistd.h>

#include "emq++.h"

#define GREEN(text) "\033[0;32m" text "\033[0;0m"
#define RED(text) "\033[0;31m" text "\033[0;0m"
#define YELLOW(text) "\033[0;33m" text "\033[0;0m"
#define MAGENTA(text) "\033[0;35m" text "\033[0;0m"

#define CHECK_STATUS(text, status) \
	if (status) \
		std::cout << GREEN("[Success]") << " " << text << std::endl; \
	else \
		std::cout << RED("[Error]") << " " << text << std::endl;

#define ADDR "localhost"

#define MESSAGES 10

static int message_counter = 0;

void worker(void)
{
	EMQ::Client client(ADDR, EMQ_DEFAULT_PORT);
	const std::string &test_message = "Hello EagleMQ";
	EMQ::Message message((void*)test_message.c_str(), test_message.length() + 1, true);
	bool status;
	int i;

	if (client.connected())
	{
		std::cout << YELLOW("[Success]") << " [Worker] Connected to "
			<< ADDR << ":" << EMQ_DEFAULT_PORT << std::endl;

		status = client.auth("eagle", "eagle");
		CHECK_STATUS("[Worker] Auth", status);

		status = client.queue.declare(".queue-test");
		CHECK_STATUS("[Worker] Queue declare", status);

		for (i = 0; i < MESSAGES; i++)
		{
			status = client.queue.push(".queue-test", message);
			CHECK_STATUS("[Worker] Queue push", status);
			sleep(1);
		}

		client.disconnect();
	}
	else
	{
		std::cout << RED("[Error]") << " Error connect to " << ADDR << ":" << EMQ_DEFAULT_PORT << std::endl;
	}
}

int queue_message_callback(emq_client *_client, int, const char *name, const char *, const char *, emq_msg *msg)
{
	EMQ::Client client(_client);
	EMQ::Message message(msg);

	printf(YELLOW("[Success]") " [Event] Message \'%s\' in queue %s\n", (char*)emq_msg_data(msg), name);

	if (++message_counter >= MESSAGES)
	{
		client.set_noack_mode(true);
		std::cout << YELLOW("[Success]") << " [Event] Queue unsubscribe" << std::endl;
		client.queue.unsubscribe(".queue-test");
		client.set_noack_mode(false);
		return 1;
	}

	return 0;
}

int main(void)
{
	EMQ::Client client(ADDR, EMQ_DEFAULT_PORT);
	bool status;

	std::cout << MAGENTA("This is a simple example of using libemq++") << std::endl;

	if (!EMQ::compatible())
	{
		std::cout << RED("[Warning]") << " Used incompatible version libemq" << std::endl;
	}

	if (client.connected())
	{
		std::cout << YELLOW("[Success]") << " Connected to " << ADDR << ":" << EMQ_DEFAULT_PORT << std::endl;

		status = client.auth("eagle", "eagle");
		CHECK_STATUS("Auth", status);

		status = client.queue.create(".queue-test", 10, 100, EMQ_QUEUE_FORCE_PUSH | EMQ_QUEUE_AUTODELETE);
		CHECK_STATUS("Queue create", status);

		status = client.queue.declare(".queue-test");
		CHECK_STATUS("Queue declare", status);

		status = client.queue.subscribe(".queue-test", EMQ_QUEUE_SUBSCRIBE_MSG, queue_message_callback);
		CHECK_STATUS("Queue subscribe", status);

		std::thread thread = std::thread(worker);

		status = client.process();
		CHECK_STATUS("Channel process", status);

		thread.join();

		client.disconnect();
	}
	else
	{
		std::cout << RED("[Error]") << " Error connect to " << ADDR << ":" << EMQ_DEFAULT_PORT << std::endl;
	}

	return 0;
}
