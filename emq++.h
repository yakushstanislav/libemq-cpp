/*
   Copyright (c) 2013, Stanislav Yakush(st.yakush@yandex.ru)
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the libemq-cpp nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS3
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __EMQ_CPP_H__
#define __EMQ_CPP_H__

extern "C"
{
#include <emq/emq.h>
}

#include <iostream>
#include <vector>

#define LIBEMQ_CPP_VERSION_MAJOR 1
#define LIBEMQ_CPP_VERSION_MINOR 0

#define LIBEMQ_COMPATIBLE_VERSION 13

namespace EMQ
{

typedef emq_perm Perm;
typedef emq_time Time;
typedef emq_tag Tag;
typedef emq_status Stat;
typedef emq_user User;
typedef emq_queue Queue;
typedef emq_route Route;
typedef emq_route_key RouteKey;
typedef emq_channel Channel;
typedef emq_msg_callback Callback;

class Message
{
public:
	Message() : message(NULL)
	{
	}

	Message(void *data, size_t size, bool zero_copy = false)
	{
		message = emq_msg_create(data, size, zero_copy);
	}

	Message(emq_msg *message)
	{
		this->message = message;
	}

	~Message()
	{
		if (message)
		{
			emq_msg_release(message);
			message = NULL;
		}
	}

	void set_expire(Time time)
	{
		emq_msg_expire(message, time);
	}

	void *data()
	{
		return emq_msg_data(message);
	}

	size_t size()
	{
		return emq_msg_size(message);
	}

	Tag tag()
	{
		return emq_msg_tag(message);
	}

	emq_msg *msg()
	{
		return message;
	}

private:
	void operator=(const Message&);

private:
	emq_msg *message;
};

class Client
{
private:
	class UserControl
	{
	public:
		inline bool create(const std::string &name, const std::string &password, Perm perm)
		{
			int status = emq_user_create(client, name.c_str(), password.c_str(), perm);

			return status == EMQ_STATUS_OK;
		}

		inline bool list(std::vector<User> &list)
		{
			emq_list_iterator iter;
			emq_list_node *node;
			emq_list *users = emq_user_list(client);

			if (!users)
			{
				return false;
			}

			emq_list_rewind(users, &iter);
			while ((node = emq_list_next(&iter)) != NULL)
			{
				list.push_back(*(User*)EMQ_LIST_VALUE(node));
			}

			emq_list_release(users);

			return true;
		}

		inline bool rename(const std::string &from, const std::string &to)
		{
			int status = emq_user_rename(client, from.c_str(), to.c_str());

			return status == EMQ_STATUS_OK;
		}

		inline bool set_perm(const std::string &name, Perm perm)
		{
			int status = emq_user_set_perm(client, name.c_str(), perm);

			return status == EMQ_STATUS_OK;
		}

		inline bool remove(const std::string &name)
		{
			int status = emq_user_delete(client, name.c_str());

			return status == EMQ_STATUS_OK;
		}

	private:
		void set_client(emq_client *client)
		{
			this->client = client;
		}

		friend Client;

	private:
		emq_client *client;
	};

	class QueueControl
	{
	public:
		inline bool create(const std::string &name, uint32_t max_msg, uint32_t max_msg_size, uint32_t flags)
		{
			int status = emq_queue_create(client, name.c_str(), max_msg, max_msg_size, flags);

			return status == EMQ_STATUS_OK;
		}

		inline bool declare(const std::string &name)
		{
			int status = emq_queue_declare(client, name.c_str());

			return status == EMQ_STATUS_OK;
		}

		inline bool exist(const std::string &name, int *queue_exist)
		{
			*queue_exist = emq_queue_exist(client, name.c_str());

			return EMQ_GET_STATUS(client) == EMQ_STATUS_OK;
		}

		inline bool list(std::vector<Queue> &list)
		{
			emq_list_iterator iter;
			emq_list_node *node;
			emq_list *queues = emq_queue_list(client);

			if (!queues)
			{
				return false;
			}

			emq_list_rewind(queues, &iter);
			while ((node = emq_list_next(&iter)) != NULL)
			{
				list.push_back(*(Queue*)EMQ_LIST_VALUE(node));
			}

			emq_list_release(queues);

			return true;
		}

		inline bool rename(const std::string &from, const std::string &to)
		{
			int status = emq_queue_rename(client, from.c_str(), to.c_str());

			return status == EMQ_STATUS_OK;
		}

		inline bool size(const std::string &name, int *queue_size)
		{
			*queue_size = emq_queue_size(client, name.c_str());

			return *queue_size != -1;
		}

		inline bool push(const std::string &name, Message &message)
		{
			int status = emq_queue_push(client, name.c_str(), message.msg());

			return status == EMQ_STATUS_OK;
		}

		inline Message get(const std::string &name)
		{
			emq_msg *msg = emq_queue_get(client, name.c_str());

			return msg;
		}

		inline Message pop(const std::string &name, Time timeout)
		{
			emq_msg *msg = emq_queue_pop(client, name.c_str(), timeout);

			return msg;
		}

		inline bool confirm(const std::string &name, Tag tag)
		{
			int status = emq_queue_confirm(client, name.c_str(), tag);

			return status == EMQ_STATUS_OK;
		}

		inline bool subscribe(const std::string &name, uint32_t flags, Callback callback)
		{
			int status = emq_queue_subscribe(client, name.c_str(), flags, callback);

			return status == EMQ_STATUS_OK;
		}

		inline bool unsubscribe(const std::string &name)
		{
			int status = emq_queue_unsubscribe(client, name.c_str());

			return status == EMQ_STATUS_OK;
		}

		inline bool purge(const std::string &name)
		{
			int status = emq_queue_purge(client, name.c_str());

			return status == EMQ_STATUS_OK;
		}

		inline bool remove(const std::string &name)
		{
			int status = emq_queue_delete(client, name.c_str());

			return status == EMQ_STATUS_OK;
		}

	private:
		void set_client(emq_client *client)
		{
			this->client = client;
		}

		friend Client;

	private:
		emq_client *client;
	};

	class RouteControl
	{
	public:
		inline bool create(const std::string &name, uint32_t flags)
		{
			int status = emq_route_create(client, name.c_str(), flags);

			return status == EMQ_STATUS_OK;
		}

		inline bool exist(const std::string &name, int *route_exist)
		{
			*route_exist = emq_route_exist(client, name.c_str());

			return EMQ_GET_STATUS(client) == EMQ_STATUS_OK;
		}

		inline bool list(std::vector<Route> &list)
		{
			emq_list_iterator iter;
			emq_list_node *node;
			emq_list *routes = emq_route_list(client);

			if (!routes)
			{
				return false;
			}

			emq_list_rewind(routes, &iter);
			while ((node = emq_list_next(&iter)) != NULL)
			{
				list.push_back(*(Route*)EMQ_LIST_VALUE(node));
			}

			emq_list_release(routes);

			return true;
		}

		inline bool keys(const std::string &name, std::vector<RouteKey> &list)
		{
			emq_list_iterator iter;
			emq_list_node *node;
			emq_list *keys = emq_route_keys(client, name.c_str());

			if (!keys)
			{
				return false;
			}

			emq_list_rewind(keys, &iter);
			while ((node = emq_list_next(&iter)) != NULL)
			{
				list.push_back(*(RouteKey*)EMQ_LIST_VALUE(node));
			}

			emq_list_release(keys);

			return true;
		}

		inline bool rename(const std::string &from, const std::string &to)
		{
			int status = emq_route_rename(client, from.c_str(), to.c_str());

			return status == EMQ_STATUS_OK;
		}

		inline bool bind(const std::string &name, const std::string &queue, const std::string &key)
		{
			int status = emq_route_bind(client, name.c_str(), queue.c_str(), key.c_str());

			return status == EMQ_STATUS_OK;
		}

		inline bool unbind(const std::string &name, const std::string &queue, const std::string &key)
		{
			int status = emq_route_unbind(client, name.c_str(), queue.c_str(), key.c_str());

			return status == EMQ_STATUS_OK;
		}

		inline bool push(const std::string &name, const std::string &key, Message &message)
		{
			int status = emq_route_push(client, name.c_str(), key.c_str(), message.msg());

			return status == EMQ_STATUS_OK;
		}

		inline bool remove(const std::string &name)
		{
			int status = emq_route_delete(client, name.c_str());

			return status == EMQ_STATUS_OK;
		}

	private:
		void set_client(emq_client *client)
		{
			this->client = client;
		}

		friend Client;

	private:
		emq_client *client;
	};

	class ChannelControl
	{
	public:
		inline bool create(const std::string &name, uint32_t flags)
		{
			int status = emq_channel_create(client, name.c_str(), flags);

			return status == EMQ_STATUS_OK;
		}

		inline bool exist(const std::string &name, int *channel_exist)
		{
			*channel_exist = emq_channel_exist(client, name.c_str());

			return EMQ_GET_STATUS(client) == EMQ_STATUS_OK;
		}

		inline bool list(std::vector<Channel> &list)
		{
			emq_list_iterator iter;
			emq_list_node *node;
			emq_list *channels = emq_channel_list(client);

			if (!channels)
			{
				return false;
			}

			emq_list_rewind(channels, &iter);
			while ((node = emq_list_next(&iter)) != NULL)
			{
				list.push_back(*(Channel*)EMQ_LIST_VALUE(node));
			}

			emq_list_release(channels);

			return true;
		}

		inline bool rename(const std::string &from, const std::string &to)
		{
			int status = emq_channel_rename(client, from.c_str(), to.c_str());

			return status == EMQ_STATUS_OK;
		}

		inline bool publish(const std::string &name, const std::string &topic, Message &message)
		{
			int status = emq_channel_publish(client, name.c_str(), topic.c_str(), message.msg());

			return status == EMQ_STATUS_OK;
		}

		inline bool subscribe(const std::string &name, const std::string &topic, Callback callback)
		{
			int status = emq_channel_subscribe(client, name.c_str(), topic.c_str(), callback);

			return status == EMQ_STATUS_OK;
		}

		inline bool psubscribe(const std::string &name, const std::string &pattern, Callback callback)
		{
			int status = emq_channel_psubscribe(client, name.c_str(), pattern.c_str(), callback);

			return status == EMQ_STATUS_OK;
		}

		inline bool unsubscribe(const std::string &name, const std::string &topic)
		{
			int status = emq_channel_unsubscribe(client, name.c_str(), topic.c_str());

			return status == EMQ_STATUS_OK;
		}

		inline bool punsubscribe(const std::string &name, const std::string &pattern)
		{
			int status = emq_channel_punsubscribe(client, name.c_str(), pattern.c_str());

			return status == EMQ_STATUS_OK;
		}

		inline bool remove(const std::string &name)
		{
			int status = emq_channel_delete(client, name.c_str());

			return status == EMQ_STATUS_OK;
		}

	private:
		void set_client(emq_client *client)
		{
			this->client = client;
		}

		friend Client;

	private:
		emq_client *client;
	};

public:
	Client(emq_client *client)
	{
		this->client = client;
		init();
	}

	Client(const std::string &addr, int port)
	{
		client = emq_tcp_connect(addr.c_str(), port);
		init();
	}

	Client(const std::string &path)
	{
		client = emq_unix_connect(path.c_str());
		init();
	}

	bool connected()
	{
		return client != NULL;
	}

	inline bool auth(const std::string &name, const std::string &password)
	{
		int status = emq_auth(client, name.c_str(), password.c_str());

		return status == EMQ_STATUS_OK;
	}

	inline bool ping()
	{
		int status = emq_ping(client);

		return status == EMQ_STATUS_OK;
	}

	inline bool status(Stat *stat)
	{
		int status = emq_stat(client, stat);

		return status == EMQ_STATUS_OK;
	}

	inline bool save(bool async)
	{
		int status = emq_save(client, async);

		return status == EMQ_STATUS_OK;
	}

	inline bool flush(uint32_t flags)
	{
		int status = emq_flush(client, flags);

		return status == EMQ_STATUS_OK;
	}

	inline void disconnect()
	{
		if (client)
		{
			emq_disconnect(client);
			client = NULL;
		}
	}

	inline int process()
	{
		int status = emq_process(client);

		return status == EMQ_STATUS_OK;
	}

	inline void set_noack_mode(bool mode)
	{
		if (mode)
		{
			emq_noack_enable(client);
		}
		else
		{
			emq_noack_disable(client);
		}
	}

	inline std::string last_error()
	{
		return emq_last_error(client);
	}

private:
	void init()
	{
		user.set_client(client);
		queue.set_client(client);
		route.set_client(client);
		channel.set_client(client);
	}

public:
	UserControl user;
	QueueControl queue;
	RouteControl route;
	ChannelControl channel;

private:
	Client(const Client&);
	void operator=(const Client&);

private:
	emq_client *client;
};

static bool compatible()
{
	return emq_version() == LIBEMQ_COMPATIBLE_VERSION;
}

};

#endif
