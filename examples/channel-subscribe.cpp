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

		for (i = 0; i < MESSAGES / 2; i++)
		{
			status = client.channel.publish(".channel-test", "world.russia.moscow", message);
			CHECK_STATUS("[Worker] Channel publish", status);
			status = client.channel.publish(".channel-test", "world.belarus.minsk", message);
			CHECK_STATUS("[Worker] Channel publish", status);
			sleep(1);
		}

		client.disconnect();
	}
	else
	{
		std::cout << RED("[Error]") << " Error connect to " << ADDR << ":" << EMQ_DEFAULT_PORT << std::endl;
	}
}

int channel_message_callback(emq_client *_client, int /*type*/, const char *name,
	const char *topic, const char *pattern, emq_msg *msg)
{
	EMQ::Client client(_client);
	EMQ::Message message(msg);

	printf(YELLOW("[Success]") " [Event] Message \'%s\' (channel: %s, pattern: %s, topic: %s) \n",
		(char*)message.data(), name, pattern, topic);

	if (++message_counter >= MESSAGES)
	{
		client.set_noack_mode(true);
		std::cout << YELLOW("[Success]") << " [Event] Channel punsubscribe" << std::endl;
		client.channel.punsubscribe(".channel-test", "world.belarus.*");
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

		status = client.channel.create(".channel-test", EMQ_CHANNEL_AUTODELETE | EMQ_CHANNEL_ROUND_ROBIN);
		CHECK_STATUS("Channel create", status);

		status = client.channel.psubscribe(".channel-test", "world.belarus.*", channel_message_callback);
		CHECK_STATUS("Channel psubscribe", status);

		std::thread thread1 = std::thread(worker);
		std::thread thread2 = std::thread(worker);

		status = client.process();
		CHECK_STATUS("Channel process", status);

		thread1.join();
		thread2.join();

		client.disconnect();
	}
	else
	{
		std::cout << RED("[Error]") << " Error connect to " << ADDR << ":" << EMQ_DEFAULT_PORT << std::endl;
	}

	return 0;
}
