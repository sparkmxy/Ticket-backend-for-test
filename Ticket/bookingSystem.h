#pragma once

#include "train.h"
#include "ticket.h"
#include "user.h"
#include "log.h"
#include "tool.h"
#include "exceptions.h"

using std::endl;

class ticketBookingSystem{
	
	trainSystem Train;
	ticketSystem Ticket;
	userSystem User;
	purchaseLog Log;
	std::ostream &os;


public:
	ticketBookingSystem(std::ostream &_os = std::cout):os(_os){
		Ticket.init(&Train);
	}

	void process(const std::string &cmd, const vector< std::pair<TYPE, String> > &V);

private:
	void Register(const vector<token> &V);

	void profile(const vector<token> &V);

	void modifyProfile(const vector<token> &V);

	void modifyType(const vector<token> &V);

	void queryTicket(const vector<token> &V);

	void transfer(const vector<token> &V);

	void buy(const vector<token> &V);

	void queryOrder(const vector<token> &V);

	void refund(const vector<token> &V);

	void addTrain(const vector<token> &V);

	void sale(const vector<token> &V);

	void queryTrain(const vector<token> &V);

	void remove(const vector<token> &V);

	void modifyTrain(const vector<token> &V);

	void clear();

	void login(const vector<token>& V);

	void Exit();
	/**************************************************************************/
	void outputInOrder(const vector<ticket> &V);
};