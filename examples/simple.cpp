#include <iostream>
#include <string>

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

static void basic(EMQ::Client &client)
{
	EMQ::Stat stat;

	CHECK_STATUS("Auth", client.auth("eagle", "eagle"));
	CHECK_STATUS("Ping", client.ping())
	CHECK_STATUS("Stat", client.status(&stat));

	std::cout << YELLOW("[Status]") << std::endl;
	std::cout << "Version" << std::endl;
	std::cout << " Major: " << (int)stat.version.major << std::endl;
	std::cout << " Minor: " << (int)stat.version.minor << std::endl;
	std::cout << " Patch: " << (int)stat.version.patch << std::endl;
	std::cout << "Uptime: " << stat.uptime << std::endl;
	std::cout << "CPU sys: " << stat.used_cpu_sys << std::endl;
	std::cout << "CPU user: " << stat.used_cpu_user << std::endl;
	std::cout << "Memory: " << stat.used_memory << std::endl;
	std::cout << "Memory RSS: " << stat.used_memory_rss << std::endl;
	std::cout << "Memory fragmentation: " << stat.fragmentation_ratio << std::endl;
	std::cout << "Clients: " << stat.clients << std::endl;
	std::cout << "Users: " << stat.users << std::endl;
	std::cout << "Queues: " << stat.queues << std::endl;
	std::cout << "Routes: " << stat.routes << std::endl;
	std::cout << "Channels: " << stat.channels << std::endl;
}

static void user_management(EMQ::Client &client)
{
	std::vector<EMQ::User> users;
	bool status;

	status = client.user.create("first.user", "password", EMQ_QUEUE_PERM);
	CHECK_STATUS("User create", status);

	status = client.user.create("user_2", "password", EMQ_QUEUE_PERM);
	CHECK_STATUS("User create", status);

	status = client.user.list(users);
	CHECK_STATUS("User list", status);

	if (status)
	{
		for (size_t i = 0; i < users.size(); i++)
		{
			const EMQ::User &user = users[i];

			std::cout << "Username: " << user.name << std::endl;
			std::cout << "Password: " << user.password << std::endl;
			std::cout << "Permissions: " << user.perm << std::endl;
			std::cout << "-----------------------" << std::endl;
		}
	}

	status = client.user.rename("user_2", "user");
	CHECK_STATUS("User rename", status);

	status = client.user.set_perm("user", EMQ_QUEUE_PERM | EMQ_ADMIN_PERM);
	CHECK_STATUS("User set perm", status);

	status = client.user.remove("first.user");
	CHECK_STATUS("User delete", status);

	status = client.user.remove("user");
	CHECK_STATUS("User delete", status);
}

static void queue_management(EMQ::Client &client)
{
	const std::string test_message = "test message";
	std::vector<EMQ::Queue> queues;
	int queue_exist, queue_size;
	bool status;

	status = client.queue.create(".queue_1", EMQ_MAX_MSG, EMQ_MAX_MSG_SIZE, EMQ_QUEUE_NONE);
	CHECK_STATUS("Queue create", status);

	status = client.queue.create("queue-2", EMQ_MAX_MSG/2, EMQ_MAX_MSG_SIZE/100, EMQ_QUEUE_NONE);
	CHECK_STATUS("Queue create", status);

	status = client.queue.declare(".queue_1");
	CHECK_STATUS("Queue declare", status);

	status = client.queue.declare("queue-2");
	CHECK_STATUS("Queue declare", status);

	status = client.queue.exist(".queue_1", &queue_exist);
	std::cout << YELLOW("[Success]") << " Queue \".queue_1\" exist: " << queue_exist << std::endl;

	status = client.queue.exist("not-exist-queue", &queue_exist);
	std::cout << YELLOW("[Success]") << " Queue \"not-exist-queue\" exist: " << queue_exist << std::endl;

	status = client.queue.list(queues);
	CHECK_STATUS("Queue list", status);

	if (status)
	{
		for (size_t i = 0; i < queues.size(); i++)
		{
			const EMQ::Queue &queue = queues[i];

			std::cout << "Name: " << queue.name << std::endl;
			std::cout << "Max msg: " << queue.max_msg << std::endl;
			std::cout << "Max msg size: " << queue.max_msg_size << std::endl;
			std::cout << "Flags: " << queue.flags << std::endl;
			std::cout << "Size: " << queue.size << std::endl;
			std::cout << "Declared clients: " << queue.declared_clients << std::endl;
			std::cout << "Subscribed clients: " << queue.subscribed_clients << std::endl;
			std::cout << "-----------------------" << std::endl;
		}
	}

	status = client.queue.rename(".queue_1", ".queue_test");
	CHECK_STATUS("Queue rename", status);

	EMQ::Message message((void*)test_message.c_str(), test_message.length() + 1, true);

	if (!message.msg())
	{
		std::cout << RED("[Error]") << " emq_msg_create" << std::endl;
		exit(1);
	}

	for (int i = 0; i < 5; i++)
	{
		status = client.queue.push(".queue_test", message);
		CHECK_STATUS("Queue push", status);

		status = client.queue.push("queue-2", message);
		CHECK_STATUS("Queue push", status);
	}

	status = client.queue.size(".queue_test", &queue_size);
	std::cout << YELLOW("[Success]") << " Queue \".queue_test\" size: " << queue_size << std::endl;

	for (int i = 0; i < 5; i++)
	{
		EMQ::Message message = client.queue.get(".queue_test");

		if (message.msg())
		{
			std::cout << YELLOW("[Success]") << " Get message: " <<
				(char*)message.data() << "(" << message.size() << ")" << std::endl;
		}
	}

	status = client.queue.size(".queue_test", &queue_size);
	std::cout << YELLOW("[Success]") << " Queue \".queue_test\" size: " << queue_size << std::endl;

	status = client.queue.size("queue-2", &queue_size);
	std::cout << YELLOW("[Success]") << " Queue \"queue-2\" size: " << queue_size << std::endl;

	for (int i = 0; i < 5; i++)
	{
		EMQ::Message message = client.queue.pop("queue-2", 1000);

		if (message.msg())
		{
			std::cout << YELLOW("[Success]") << " Pop message: " <<
				(char*)message.data() << "(" << message.size() << ")" << std::endl;

			status = client.queue.confirm("queue-2", message.tag());
			CHECK_STATUS("Queue confirm", status);
		}
	}

	status = client.queue.size("queue-2", &queue_size);
	std::cout << YELLOW("[Success]") << " Queue \"queue-2\" size: " << queue_size << std::endl;

	status = client.queue.purge(".queue_test");
	CHECK_STATUS("Queue purge", status);

	status = client.queue.size(".queue_test", &queue_size);
	std::cout << YELLOW("[Success]") << " Queue \".queue_test\" size: " << queue_size << std::endl;

	status = client.queue.remove(".queue_test");
	CHECK_STATUS("Queue delete", status);

	status = client.queue.remove("queue-2");
	CHECK_STATUS("Queue delete", status);
}

static void route_management(EMQ::Client &client)
{
	const std::string test_message = "test message";
	std::vector<EMQ::Route> routes;
	std::vector<EMQ::RouteKey> route_keys;
	int route_exist;
	int queue_size;
	bool status;

	status = client.route.create(".route_1", EMQ_ROUTE_NONE);
	CHECK_STATUS("Route create", status);

	status = client.queue.create(".queue_1", EMQ_MAX_MSG, EMQ_MAX_MSG_SIZE, EMQ_QUEUE_NONE);
	CHECK_STATUS("Queue create", status);

	status = client.queue.create(".queue_2", EMQ_MAX_MSG, EMQ_MAX_MSG_SIZE, EMQ_QUEUE_NONE);
	CHECK_STATUS("Queue create", status);

	status = client.route.exist(".route_1", &route_exist);
	std::cout << YELLOW("[Success]") << " Route \".route_1\" exist: " << route_exist << std::endl;

	status = client.route.exist("not-exist-route", &route_exist);
	std::cout << YELLOW("[Success]") << " Route \"not-exist-route\" exist: " << route_exist << std::endl;

	status = client.route.list(routes);
	CHECK_STATUS("Route list", status);

	if (status)
	{
		for (size_t i = 0; i < routes.size(); i++)
		{
			const EMQ::Route &route = routes[i];

			std::cout << "Name: " << route.name << std::endl;
			std::cout << "Flags: " << route.flags << std::endl;
			std::cout << "Keys: " << route.keys << std::endl;
			std::cout << "-----------------------" << std::endl;
		}
	}

	status = client.route.bind(".route_1", ".queue_1", "key1");
	CHECK_STATUS("Route bind", status);

	status = client.route.bind(".route_1", ".queue_2", "key1");
	CHECK_STATUS("Route bind", status);

	status = client.route.bind(".route_1", ".queue_2", "key2");
	CHECK_STATUS("Route bind", status);

	status = client.route.keys(".route_1", route_keys);
	CHECK_STATUS("Route keys", status);

	if (status)
	{
		for (size_t i = 0; i < route_keys.size(); i++)
		{
			const EMQ::RouteKey &key = route_keys[i];

			std::cout << "Key: " << key.key << std::endl;
			std::cout << "Queue: " << key.queue << std::endl;
			std::cout << "-----------------------" << std::endl;
		}
	}

	status = client.route.rename(".route_1", ".route_test");
	CHECK_STATUS("Route rename", status);

	EMQ::Message message((void*)test_message.c_str(), test_message.length() + 1, true);

	if (!message.msg())
	{
		std::cout << RED("[Error]") << " emq_msg_create" << std::endl;
		exit(1);
	}

	for (int i = 0; i < 5; i++)
	{
		status = client.route.push(".route_test", "key1", message);
		CHECK_STATUS("Route push", status);

		status = client.route.push(".route_test", "key2", message);
		CHECK_STATUS("Route push", status);
	}

	status = client.queue.size(".queue_1", &queue_size);
	std::cout << YELLOW("[Success]") << " Queue \".queue_1\" size: " << queue_size << std::endl;

	status = client.queue.size(".queue_2", &queue_size);
	std::cout << YELLOW("[Success]") << " Queue \".queue_2\" size: " << queue_size << std::endl;

	status = client.route.unbind(".route_test", ".queue_1", "key1");
	CHECK_STATUS("Route unbind", status);

	status = client.route.unbind(".route_test", ".queue_2", "key1");
	CHECK_STATUS("Route unbind", status);

	status = client.route.unbind(".route_test", ".queue_2", "key2");
	CHECK_STATUS("Route unbind", status);

	status = client.route.remove(".route_test");
	CHECK_STATUS("Route delete", status);

	status = client.queue.remove(".queue_1");
	CHECK_STATUS("Queue delete", status);

	status = client.queue.remove(".queue_2");
	CHECK_STATUS("Queue delete", status);
}

static void channel_management(EMQ::Client &client)
{
	std::vector<EMQ::Channel> channels;
	int channel_exist;
	bool status;

	status = client.channel.create(".channel_1", EMQ_CHANNEL_NONE);
	CHECK_STATUS("Channel create", status);

	status = client.channel.create(".channel_2", EMQ_CHANNEL_NONE);
	CHECK_STATUS("Channel create", status);

	status = client.channel.exist(".channel_1", &channel_exist);
	std::cout << YELLOW("[Success]") << " Channel \".channel_1\" exist: " << channel_exist << std::endl;

	status = client.channel.exist(".not-exist-channel", &channel_exist);
	std::cout << YELLOW("[Success]") << " Channel \"not-exist-channel\" exist: " << channel_exist << std::endl;

	status = client.channel.list(channels);
	CHECK_STATUS("Channel list", status);

	if (status)
	{
		for (size_t i = 0; i < channels.size(); i++)
		{
			const EMQ::Channel &channel = channels[i];

			std::cout << "Name: " << channel.name << std::endl;
			std::cout << "Flags: " << channel.flags << std::endl;
			std::cout << "Topics: " << channel.topics << std::endl;
			std::cout << "Patterns: " << channel.patterns << std::endl;
			std::cout << "-----------------------" << std::endl;
		}
	}

	status = client.channel.remove(".channel_1");
	CHECK_STATUS("Channel delete", status);

	status = client.channel.remove(".channel_2");
	CHECK_STATUS("Channel delete", status);
}

int main(void)
{
	EMQ::Client client(ADDR, EMQ_DEFAULT_PORT);

	std::cout << MAGENTA("This is a simple example of using libemq++") << std::endl;

	if (!EMQ::compatible())
	{
		std::cout << RED("[Warning]") << " Used incompatible version libemq" << std::endl;
	}

	if (client.connected())
	{
		std::cout << YELLOW("[Success]") << " Connected to " << ADDR << ":" << EMQ_DEFAULT_PORT << std::endl;

		basic(client);
		user_management(client);
		queue_management(client);
		route_management(client);
		channel_management(client);

		client.disconnect();
	}
	else
	{
		std::cout << RED("[Error]") << " Error connect to " << ADDR << ":" << EMQ_DEFAULT_PORT << std::endl;
	}

	return 0;
}
