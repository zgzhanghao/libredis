#pragma once
#include "all.h"
#include "hiredis.h"
#include "tcpserver.h"
#include "proxysession.h"

class RedisProxy
{
public:
	RedisProxy(const char *ip, int16_t port, const char *redisIp,
		int16_t redisPort, int16_t threadCount, int16_t sessionCount);
	~RedisProxy();

	void proxyConnCallback(const TcpConnectionPtr &conn);
	void processCommand(const TcpConnectionPtr &conn, const char *buf, size_t len);
	void run();

	void initRedisPorxy();
	void initRedisAsync();
	void initRedisCommand();
	void initRedisTimer();
	RedisAsyncContextPtr checkReply(const TcpConnectionPtr &conn);

	bool getRedisCommand(const RedisObjectPtr &command);
	bool handleRedisCommand(const RedisObjectPtr &command,
		const ProxySessionPtr &session,
		const std::vector<RedisObjectPtr> &commands,
		const TcpConnectionPtr &conn);
	bool hgetallCommand(const std::vector<RedisObjectPtr> &commands,
		const ProxySessionPtr &session, const TcpConnectionPtr &conn);
	bool mgetCommand(const std::vector<RedisObjectPtr> &commands,
		const ProxySessionPtr &session, const TcpConnectionPtr &conn);

	void highWaterCallback(const TcpConnectionPtr &conn, size_t bytesToSent);
	void writeCompleteCallback(const TcpConnectionPtr &conn);
	void redisConnCallback(const TcpConnectionPtr &conn);
	void redisDisconnCallback(const TcpConnectionPtr &conn);
	void proxyCallback(const RedisAsyncContextPtr &c,
		const RedisReplyPtr &reply, const std::any &privdata);

	void clearProxy(const std::thread::id &threadId, const int32_t sockfd);
	void clearProxyReply(const std::thread::id &threadId, const int32_t sockfd);
	void clearProxyCount(const std::thread::id &threadId, const int32_t sockfd);
	void clearProxySend(const std::thread::id &threadId, const int32_t sockfd);
	void clearProxyRedis(const std::thread::id &threadId, const int32_t sockfd);
	void clearProxyRedisClient(const std::thread::id &threadId, const int32_t sockfd);
	void eraseProxySend(const std::thread::id &threadId, const int32_t sockfd, const int64_t count);
	void foreachProxyReply(const std::thread::id &threadId, const int32_t sockfd, int64_t begin);
	void insertProxyReply(const std::thread::id &threadId,
		const int32_t sockfd, const int64_t count, const RedisReplyPtr &reply);
	int64_t insertProxyCount(const std::thread::id &threadId, const int32_t sockfd);
	void insertProxySend(const std::thread::id &threadId, const int32_t sockfd, const int64_t count);
	void insertCommandReply(const std::thread::id &threadId, const int32_t sockfd, const RedisReplyPtr &reply);
	void clearCommandReply(const std::thread::id &threadId, const int32_t sockfd);
	int32_t getCommandReplyCount(const std::thread::id &threadId, const int32_t sockfd);
	void processCommandReply(const std::thread::id &threadId, const int32_t sockfd, const TcpConnectionPtr &conn);

private:
	EventLoop loop;
	TcpServer server;
	std::mutex mutex;

	const char *ip;
	const char *redisIp;
	int16_t port;
	int16_t redisPort;
	int16_t threadCount;
	int16_t sessionCount;

	static const int32_t kHeart = 10;
	static const int32_t kHighWaterBytes = 1024 * 1024 * 64;

	std::unordered_map<int32_t, ProxySessionPtr> sessions;
	std::unordered_map<std::thread::id, std::shared_ptr<Hiredis>> threadHiredis;
	std::unordered_map<std::thread::id, std::vector<RedisContextPtr>> threadRedisContexts;
	std::unordered_map<std::thread::id, std::unordered_map<int32_t, std::map<int64_t, RedisReplyPtr>>> proxyReplys;
	std::unordered_map<std::thread::id, std::unordered_map<int32_t, std::set<int64_t>>> proxySends;
	std::unordered_map<std::thread::id, std::unordered_map<int32_t, std::vector<RedisReplyPtr>>> commandReplys;
	std::unordered_map<std::thread::id, std::unordered_map<int32_t, int64_t>> proxyCounts;
	std::unordered_map<std::thread::id, std::unordered_map<int32_t, std::unordered_set<int32_t>>> proxyRedis;
	std::unordered_map<std::thread::id, std::vector<RedisObjectPtr>> proxyCommands;
	typedef std::function<bool(const std::vector<RedisObjectPtr> &,
		const ProxySessionPtr &, const TcpConnectionPtr &)> CommandFunc;
	std::unordered_map<RedisObjectPtr, CommandFunc, Hash, Equal> redisCommands;
};
