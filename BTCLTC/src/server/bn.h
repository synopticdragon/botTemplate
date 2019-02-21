#include <fstream>
#include <sstream>
#include <iostream>
#include <curl/curl.h>
#include <string>
#include <time.h>
#include <stdio.h>
#include "quickfix/NullStore.h"
#include "quickfix/Application.h"
#include "quickfix/SocketInitiator.h"
#include "quickfix/SessionSettings.h"
#include "quickfix/fix42/NewOrderSingle.h"
#include "quickfix/fix42/OrderCancelRequest.h"
#include "quickfix/fix42/ExecutionReport.h"

		const std::string apiUrl("https://api.bitfinex.com/v1/ticker/btcusd");
//"https://api.zaif.jp/api/1/ticker/btc_jpy");

		static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
		{
			((std::string*)userp)->append((char*)contents, size * nmemb);
			return size * nmemb;
		}
		static int counter = 1;
		static const double david=1;
		static const string exName = "BEAXCHANGE";

		double cutter(string &data)
		{
			int pos = data.find("\"");
			string ans;

			data.erase(data.begin(), data.begin() + (pos +1));

			for(int a=0; a < data.length(); a++)
			{
				if(data[a] ==  '"')
				{
					data.erase(data.begin(), data.begin() + a +2);
					int next=a +1, next1 = next +1;
					if(data[next] == ',' && isdigit(data[next1]))
					{
						pos= data.find("[");
						data.erase(data.begin(), data.begin()+ pos +1);
					}
					break;
				}
				ans.push_back(data[a]);
				
			}
			std::string::size_type size;
			double result = stod(ans, &size);
			std::cout.precision(6);
			return result;
			// cout<<"\n"<<data;
		}
		///	binance
		double cutter(string &data, char key)
		{
		string ans;

		for(int a=0; a <data.length(); a++)
		{
			if(data[a] ==  '"')
			{
				if(key==']')
				{
				    int pos= data.find(key);
					data.erase(data.begin(), data.begin() + pos+5);
					break;
				}else{		   
					data.erase(data.begin(), data.begin() + a+3);
					break;
					}
				
			}
			
			ans.push_back(data[a]);
				
			}
			//cout<<endl<<ans<<endl;
			cout<<fixed;
			std::string::size_type size;
			double result = stod(ans, &size);
			return result;
		}
	
		void syncFile(string synctrade)
		{
			ofstream execTrades ("Trades",ios::app);
	
			if(execTrades.is_open())
			{
				execTrades << synctrade;
				execTrades.close();
			}
			else
			{
				cerr << "\n\n====UNABLE TO SAVE=====\n\n" <<endl;
			}		
			
		}
		static void fileToClear(string filename, string key)
		{
			ofstream execTrades (filename,ios::trunc);
			ofstream tocancel("second",ios::trunc);
	
			if(execTrades.is_open())
			{
				execTrades << key;
				execTrades.close();
			}
			else
			{
				cerr << "\n\n====UNABLE TO clear trades on EXIT=====\n\n" <<endl;
			}	
			
			if(tocancel.is_open())
			{
				tocancel << "dashweh";
				tocancel.close();
			}
			else
			{
				cerr << "\n\n====UNABLE TO clear trades on EXIT=====\n\n" <<endl;
			}	
			
		}
		void cancelFile(string trade)
		{
			ofstream execTrades ("cancelTrades",ios::app);
	
			if(execTrades.is_open())
			{
				execTrades << trade;
				execTrades.close();
			}
			else
			{
				cerr << "\n\n====UNABLE TO SAVE=====\n\n" <<endl;
			}		
			
		}

		void sendFixPeatio(string tradeSide, string tradePrice, string orderQnty, string tradeTime, string crncyTrade, string trTradeId)
		{
			
			/*FIX42::ExecutionReport botTrade = FIX42::ExecutionReport();				
			botTrade.setField(56, "Peatio"); //peatio
			botTrade.setField(49, "TradingBot");
			//Body of fix message
			botTrade.setField(54, tradeSide); // buy or sell
			botTrade.setField(60, tradeTime);//timestamp
			botTrade.setField(38, orderQnty); // amount traded
			botTrade.setField(44, tradePrice); //traded price
			botTrade.setField(15, crncyTrade); //base traded currency BTC-->EUR pair.base
			botTrade.setField(40, "1"); // hardcode market
			botTrade.setField(58, trTradeId);
			botTrade.setField(39, "2"); // URL used to tell to trade
			
			//print to console to test if fix message was successful
			try{				
							
				stringstream fixTrade;
				fixTrade << botTrade << endl;
				//cancelFile(fixTrade.str());

				//cout<< botTrade <<endl;
			      //cout<<"\nTRADED  \n";
						

			}catch(...)
			{
				cout<< "\n=================\nFAILED TO CREATE FIX REPORT\n========================="<<endl;
			}*/
			
			
		
		}

		void cancelsendFixPeatio(string cnclOrid, string cnclSide, string cncltime, string cnclprice, string cnclquantity, string cnclcurency)
		{
			
			FIX42::ExecutionReport botTrade = FIX42::ExecutionReport();				
			botTrade.setField(56, "Peatio"); //peatio
			botTrade.setField(49, "TradingBot");
			//Body of fix message
			botTrade.setField(54, cnclSide); // buy or sell
			botTrade.setField(60, cncltime);//timestamp
			botTrade.setField(38, cnclquantity); // amount traded
			botTrade.setField(44, cnclprice); //traded price
			botTrade.setField(40, "2"); // hardcode market
			botTrade.setField(15, cnclcurency); //base traded currency BTC-->EUR pair.base
			botTrade.setField(11, cnclOrid);
			botTrade.setField(39, "4"); //URL used to tell if cancel or trade
			
			//print to console to test if fix message was successful
			try{				
						
				stringstream fixTrade;
				fixTrade << botTrade << endl;
				cancelFile(fixTrade.str());				


				cout<<" \n=====Cancel Trade======= \n" <<botTrade <<endl;
						

			}catch(...)
			{
				cout<< "\n=================\nFAILED TO CREATE FIX REPORT\n========================="<<endl;
			}
			
			
		
		}
		void sonaliSync(string syncOrid, string syncSide, string syncTime, string syncPrice, string syncQuantity, string syncCurency)
		{
			FIX42::ExecutionReport syncTrade = FIX42::ExecutionReport();				
			syncTrade.setField(56, "Peatio"); //peatio
			syncTrade.setField(49, "TradingBot");
			//Body of fix message
			syncTrade.setField(54, syncSide); // buy or sell
			syncTrade.setField(60, syncTime);//timestamp
			syncTrade.setField(38, syncQuantity); // amount traded
			syncTrade.setField(44, syncPrice); //traded price
			syncTrade.setField(40, "2"); // hardcode market
			syncTrade.setField(15, syncCurency); //base traded currency BTC-->EUR pair.base
			syncTrade.setField(41, syncOrid);
			//syncTrade.setField(11, syncReplace);
			syncTrade.setField(39, "0"); //URL used to tell if cancel or trade
			
			//print to console to test if fix message was successful
			try{				
						
				stringstream fixTrade;
				fixTrade << syncTrade << endl;
				syncFile(fixTrade.str());				

				cout<<" \n=====FIX===TRADES======= \n" << syncTrade <<endl;
										

			}catch(...)
			{
				cout<< "\n=================\nFAILED TO CREATE FIX REPORT\n========================="<<endl;
			}
			
			
		}
		//curl function to get orderbook
		std::string getOrderBook(const std::string apiUrl)
		{
			CURL *curl;
			CURLcode res;
			std::string apiData;

			 curl = curl_easy_init();
			if(curl) {
			curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &apiData);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			return apiData;
			}
		}

		double getJsonValue(std::string param, std::string json)
		{
			//std::string json = getOrderBook(apiUrl);
			std::size_t pos = json.find(param) + param.length() + 3;
			std::stringstream ssin(json);std::string result;
			while(json[pos] != '"')
			{
				result += json[pos];pos++;
			}

			std::string::size_type size;
			double ans = stod(result, &size);
			std::cout.precision(10);

			return ans;
		}
		double getJsonValue(std::string param, std::string json, int limit)
		{
			//std::string json = getOrderBook(apiUrl);
			std::size_t pos = json.find(param) + param.length() + limit;
			std::stringstream ssin(json);std::string result;
			while(json[pos] != '"')
			{
				result += json[pos];pos++;
			}
			//cout<<"\n "<<result<<endl;
			std::string::size_type size;
			double ans = stod(result, &size);
			std::cout.precision(10);

			return ans;
		}

		const std::string trTransactTime() 
		{
			time_t     now = time(0);
			struct tm  tstruct;
			char       buf[80];
			tstruct = *localtime(&now);		
			strftime(buf, sizeof(buf), "%Y%m%d-%X", &tstruct);
			return buf;
		}
		int getday()
		{
		    std::time_t t = std::time(0);   
		    std::tm* now = std::localtime(&t);    
		       int dayz =now->tm_mday;

		       return dayz;
		}
		int getminute()
		{
		    std::time_t t = std::time(0);   
		    std::tm* now = std::localtime(&t);    
		       int dayz =now->tm_min;

		       return dayz;
		}

		//static int dayz= getday();		


		
		
		void makefile(double trade, string filename)
		{
		    ofstream execTrades (filename,ios::app);
		    
			execTrades.precision(6);
		    //execTrades.setf(ios::fixed);
		    //execTrades.setf(ios::showpoint);

		    if(execTrades.is_open())
		    {
			execTrades <<trade <<endl;
			execTrades.close();
		    }
		    else
		    {
			cerr << "\n\n====UNABLE TO SAVE=====\n\n" <<endl;
		    }
		}

		double randme(double paluchimin, double paluchimax)
        {
                double t,y;
                y = (double)rand() / (RAND_MAX + 1.0);
                t=paluchimin + y *(paluchimax - paluchimin);
                cout<<fixed;
                return t;
        }	

		bool createTrades()
   	   	{
	      		bool val;
	   		string bid = getOrderBook("https://api.binance.com/api/v1/depth?symbol=LTCBTC");
			if (bid.length() < 800) 
			{
				val=false;			
			}else{
			/**/remove("askprice");remove("asksize");
			remove("bidprice");remove("bidsize");	
			bid.erase(bid.begin(), bid.begin() + 36);
			string ask = bid;
			int pos = ask.find("asks");
			ask.erase(ask.begin(), ask.begin() + pos + 9);
			
		        for(int x=0; x < 10; x++)
		        {      
				double askprice =  cutter(ask, '"'); 
				double bidprice =  cutter(bid, '"');
				double asksize  =  cutter(ask, ']');
				double bidsize  =  cutter(bid, ']');

				makefile(1/(askprice * david), "askprice"); makefile(1/(asksize * david), "asksize");
				makefile(1/(bidprice * david), "bidprice"); makefile(1/(bidsize * david), "bidsize"); 	
			
		        }                                    
		      
		       val =true;
		      }


		      return val;
        	}
		int getTime()
		{
		    std::time_t t = std::time(0);   
		    std::tm* now  = std::localtime(&t);    
		   int minute = now->tm_min;

		   return minute;
		}
		void makePaluchi()	
		{
			string val = getOrderBook("https://api.binance.com/api/v3/ticker/price?symbol=LTCBTC");
			size_t f = val.find("price");
			if(f < 99)
			{
				remove("currentprice");				
				makefile(1/(getJsonValue("price",val,3)), "currentprice");
			}			 		 
		}


	
				



