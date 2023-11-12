#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>

#include "Client.hpp"
#include "Server.hpp"

class Client;

class Channel
{
	private:
		string _channelName;
		vector<Client*> _clients;
		Client* _channelOwner;

		/* MODES */
		string _channelKey;
		size_t _channelLimit;
		bool _channelPrivate;

		Channel();
		Channel( const Channel& copy );
	public:
		Channel ( const string& channelName, const string& channelKey, Client* channelOwner );
		~Channel();

		/* GETTERS */
		string getChannelName() const { return _channelName; }
		Client* getChannelOwner() const { return _channelOwner; }

		string getChannelKey() const { return _channelKey; }
		size_t getChannelLimit() const { return _channelLimit; }
		bool isChannelPrivate() const { return _channelPrivate; }

		size_t getChannelClientCount() const { return _clients.size(); }
		vector<string> getChannelClientNickNames() const;

		/* SETTERS */
		void setChannelKey( const string& chanelKey ) { _channelKey = chanelKey; }
		void setChannelLimit( size_t chanelLimit ) { _channelLimit = chanelLimit; }
		void setChannelPrivate( bool chanelPrivate ) { _channelPrivate = chanelPrivate; }

		/* ACTIONS */
		void broadcastMessage( const string& message ) const;
		void broadcastMessage( const string& message, Client* exceptClient ) const;

		void addClient( Client* client );
		void removeClient( Client* client );
		void kickClient( Client* client, Client* target, const string& reason = "" );
};

#endif