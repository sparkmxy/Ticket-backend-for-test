#ifndef EXC_HPP
#define EXC_HPP


#define REPORT { printf("Invalid\n"); return; }

#include "tokenscanner.hpp"
#include "bookingSystem.h"
#include "tool.h"
#include "vector.hpp"
#include <string>

using std::string;

void EXECUTOR(ticketBookingSystem &B,const string &_log)
{
    TokenScanner scanner;
    scanner.setInput(_log);
    if (!scanner.hasMoreTokens()) REPORT;
    string firsttoken = scanner.nextToken();
    //user
	//std::cout << "fisrsttoken: " << firsttoken << endl;
    if (firsttoken == "register") //name password email phone
    {
        vector< std::pair<TYPE,String> > parameter;
		if (!scanner.hasMoreTokens()) {
			std::cout << "report: no more tokens.\n";
			REPORT;
		}

        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));
        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();

        parameter.push_back(std::make_pair(STRING, tmp));
        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));
        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        B.process(firsttoken, parameter);
    }
    else if (firsttoken == "login") //id password
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_INT, tmp));//is id a number?
        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

		B.process(firsttoken, parameter);
        //?
    }
    else if (firsttoken == "query_profile") //id
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_INT, tmp));
        
		B.process(firsttoken, parameter);
        //?
    }
    else if (firsttoken == "modify_profile") //id name password email phone
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_INT, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));
        
		B.process(firsttoken, parameter);
        //?
    }
    else if (firsttoken == "modify_privilege") //id1 id2 privilege
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_INT, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_INT, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        //int privilege = scanner.isNumber(tmp);
        //if (privilege == -1) REPORT;
        parameter.push_back(std::make_pair(_INT, tmp));
        
		B.process(firsttoken, parameter);
        //?
    }
    //ticket
    else if (firsttoken == "query_ticket") //loc1 loc2 date catalog
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_DATE, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));
        
		B.process(firsttoken, parameter);
        //?
    }
    else if (firsttoken == "query_transfer")//loc1 loc2 date catalog
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_DATE, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));
        
		B.process(firsttoken, parameter);
        //?
    }
    else if (firsttoken == "buy_ticket") //id num train_id loc1 loc2 _DATE kind
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_INT, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        //int num = scanner.isNumber(tmp);
        //if (num == -1) REPORT;
        parameter.push_back(std::make_pair(_INT, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));
        
        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_DATE, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));
        
		B.process(firsttoken, parameter);
        //?
    }
    else if (firsttoken == "query_order") //id date catalog
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_INT, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_DATE, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

		B.process(firsttoken, parameter);
        //?
    }
    else if (firsttoken == "refund_ticket")//id num trian_id loc1 loc2 date kind
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_INT, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        //int num = scanner.isNumber(tmp);
        //if (num == -1) REPORT;
        parameter.push_back(std::make_pair(_INT, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));
        
        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(_DATE, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));
        
		B.process(firsttoken, parameter);
        //?
    }
    //train
    //train_id name catalog num(station) num(price) name_price
    //station_name timearrive timestart timestopover priceofeachkind
    else if (firsttoken == "add_train")
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        int num_station = tmp.asint();
        parameter.push_back(std::make_pair(_INT, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        int num_price = tmp.asint();
        parameter.push_back(std::make_pair(_INT, tmp));

        for (int i=1;i<=num_price;++i)
        {
            if (!scanner.hasMoreTokens()) REPORT;
            tmp = scanner.nextToken();
            parameter.push_back(std::make_pair(STRING, tmp));
        }

        for (int _=1;_<=num_station;++_)
        {
            if (!scanner.hasMoreTokens()) REPORT;
            tmp = scanner.nextToken();
            parameter.push_back(std::make_pair(STRING, tmp));

            if (!scanner.hasMoreTokens()) REPORT;
            tmp = scanner.nextToken();
            parameter.push_back(std::make_pair(TIME, tmp));

            if (!scanner.hasMoreTokens()) REPORT;
            tmp = scanner.nextToken();
            parameter.push_back(std::make_pair(TIME, tmp));

            if (!scanner.hasMoreTokens()) REPORT;
            tmp = scanner.nextToken();
            parameter.push_back(std::make_pair(TIME, tmp));

            for (int i=1;i<=num_price;++i)
            {
                if (!scanner.hasMoreTokens()) REPORT;
                tmp = scanner.nextToken();
                parameter.push_back(std::make_pair(_DOUBLE, tmp));
            }        
        }

		B.process(firsttoken, parameter);
        //?
    }
    else if (firsttoken == "sale_train")//train_id
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));        
        
		B.process(firsttoken, parameter);
        //?
    }
    else if (firsttoken == "query_train")//train_id
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));    
        
		B.process(firsttoken, parameter);
        //?
    }
    else if (firsttoken == "delete_train")//train_id
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));    
        
		B.process(firsttoken, parameter);
        //?
    }
    //train_id name catalog num(station) num(price) name_price
    //station_name timearrive timestart timestopover priceofeachkind
    else if (firsttoken == "modify_train")
    {
        vector< std::pair<TYPE,String> > parameter;
        if (!scanner.hasMoreTokens()) REPORT;
        String tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        parameter.push_back(std::make_pair(STRING, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        int num_station = tmp.asint();
        parameter.push_back(std::make_pair(_INT, tmp));

        if (!scanner.hasMoreTokens()) REPORT;
        tmp = scanner.nextToken();
        int num_price = tmp.asint();
        parameter.push_back(std::make_pair(_INT, tmp));

        for (int i=1;i<=num_price;++i)
        {
            if (!scanner.hasMoreTokens()) REPORT;
            tmp = scanner.nextToken();
            parameter.push_back(std::make_pair(STRING, tmp));
        }

        for (int _=1;_<=num_station;++_)
        {
            if (!scanner.hasMoreTokens()) REPORT;
            tmp = scanner.nextToken();
            parameter.push_back(std::make_pair(STRING, tmp));

            if (!scanner.hasMoreTokens()) REPORT;
            tmp = scanner.nextToken();
            parameter.push_back(std::make_pair(TIME, tmp));

            if (!scanner.hasMoreTokens()) REPORT;
            tmp = scanner.nextToken();
            parameter.push_back(std::make_pair(TIME, tmp));

            if (!scanner.hasMoreTokens()) REPORT;
            tmp = scanner.nextToken();
            parameter.push_back(std::make_pair(TIME, tmp));

            for (int i=1;i<=num_price;++i)
            {
                if (!scanner.hasMoreTokens()) REPORT;
                tmp = scanner.nextToken();
                parameter.push_back(std::make_pair(_DOUBLE, tmp));
            }        
        }

		B.process(firsttoken, parameter);
        //?
    }
    //manager
    else if (firsttoken == "clean")
    {
	B.process(firsttoken, vector< std::pair<TYPE, String> >());
//the clean procedure
    }
    else if (firsttoken == "exit")
    {
	B.process(firsttoken, vector< std::pair<TYPE, String> >());
//the exit procedure
    }
    else REPORT;   
}

#endif
