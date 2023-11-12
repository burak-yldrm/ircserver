#include "../include/Channel.hpp"

Channel::Channel( const string& chanelName, const string& chanelKey, Client* chanelOwner )
	: _channelName(chanelName),
	_channelOwner(chanelOwner),
	_channelKey(chanelKey),
	_channelLimit(0),
	_channelPrivate(false)
{
}

Channel::~Channel() {}

vector<string> Channel::getChannelClientNickNames() const
{
	vector<string> nickNames;
	for (vector<Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
		string nickName = (*it == _channelOwner ? "@" : "") + (*it)->getNickName();
		nickNames.push_back(nickName);
	}

	return nickNames;
}

void Channel::broadcastMessage( const string& message ) const
{
	for (vector<Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
		(*it)->sendMessage(message);
	}
}

void Channel::broadcastMessage( const string& message, Client* exceptClient ) const
{
	for (vector<Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (*it != exceptClient) {
			(*it)->sendMessage(message);
		}
	}
}

void Channel::addClient( Client* client )
{
	_clients.push_back(client);
	broadcastMessage(":" + client->getPrefix() + " JOIN :" + _channelName, client);
}

void Channel::removeClient( Client* client )
{
	for (vector<Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (*it == client) {
			_clients.erase(it);
			return;
		}
	}
}