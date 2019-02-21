#ifndef K_GW_H_
#define K_GW_H_

#include <sstream>

namespace K {
  class GW: public Klass {
    private:
      mConnectivity gwConnected               = mConnectivity::Disconnected,
                    gwConnectedAdmin          = mConnectivity::Disconnected,
                    gwConnectedExchange       = mConnectivity::Disconnected,
                    gwConnectedExchangeOrders = mConnectivity::Disconnected,
                    gwConnectedExchangeMarket = mConnectivity::Disconnected;
      unsigned int gwT_5m        = 0,
                   gwT_countdown = 0;
      bool sync_levels = false,
           sync_trades = false,
           sync_orders = false;
    protected:
      void load() {                                                 _debugEvent_
        endingFn.back() = &happyEnding;
        gwConnectedAdmin = (mConnectivity)((CF*)config)->argAutobot;
        handshake(gw->exchange);
      };

      void waitData() {                                             _debugEvent_
        ((QE*)engine)->gwConnected =
        ((SH*)screen)->gwConnected = &gwConnected;
        ((QE*)engine)->gwConnectedExchange =
        ((SH*)screen)->gwConnectedExchange = &gwConnectedExchange;
        gw->reconnect = [&](string reason) {
          gwConnect(reason);
        };
        gw->evConnectOrder = [&](mConnectivity k) {
          gwSemaphore(&gwConnectedExchangeOrders, k);
        };
        gw->evConnectMarket = [&](mConnectivity k) {
          if (!gwSemaphore(&gwConnectedExchangeMarket, k))
            gw->evDataLevels(mLevels());
            //gw->evDataLevels(mLevels());
        };
      };
      void waitTime() {                                             _debugEvent_
        sync_levels = !gw->async_levels();
        sync_trades = !gw->async_trades();
        sync_orders = !gw->async_orders();
        if (!sync_levels) gwConnect();
        ((EV*)events)->tServer->setData(this);
        ((EV*)events)->tServer->start([](Timer *tServer) {
          ((GW*)tServer->getData())->timer_1s();
        }, 0, 1e+3);
      };
      void waitUser() {                                             _debugEvent_
        ((UI*)client)->welcome(mMatter::Connectivity, &hello);
        ((UI*)client)->clickme(mMatter::Connectivity, &kiss);
        ((SH*)screen)->pressme(mHotkey::ESC, &hotkiss);
      };
      void run() {                                                  _debugEvent_
        ((EV*)events)->start();
      };
    private:
      function<void()> happyEnding = [&]() {
        ((EV*)events)->stop([&]() {
	 fileToClear("clear", "dashweh");
          if (((CF*)config)->argDustybot)
            ((SH*)screen)->log(string("GW ") + exName,/*gw->name,*/ "--dustybot is enabled, remember to cancel manually any open order.");
          else {
            ((SH*)screen)->log(string("GW ") + exName,/*gw->name,*/ "Attempting to cancel all open orders, please wait. \n");
            for (mOrder &it : gw->sync_cancelAll()) gw->evDataOrder(it);           
            ((SH*)screen)->log(string("GW ") + exName,/*gw->name,*/ "cancel all open orders OK");

		fileToClear("clear", "dashweh");
          }
          if (gw->exchange == mExchange::Coinbase) stunnel();
        });
      };
      function<void(json*)> hello = [&](json *welcome) {
        *welcome = { semaphore() };
      };
      function<void(json)> kiss = [&](json butterfly) {
        if (!butterfly.is_object() or !butterfly["state"].is_number()) return;
        mConnectivity updated = butterfly["state"].get<mConnectivity>();
        if (gwConnectedAdmin != updated) {
          gwConnectedAdmin = updated;
          gwAdminSemaphore();
        }
      };
      function<void()> hotkiss = [&]() {
        gwConnectedAdmin = (mConnectivity)!gwConnectedAdmin;
        gwAdminSemaphore();
      };
      mConnectivity gwSemaphore(mConnectivity *current, mConnectivity updated) {
        if (*current != updated) {
          *current = updated;
          gwConnectedExchange = gwConnectedExchangeMarket * gwConnectedExchangeOrders;
          gwAdminSemaphore();
        }
        return updated;
      };
      void gwAdminSemaphore() {
        mConnectivity updated = gwConnectedAdmin * gwConnectedExchange;
        if (gwConnected != updated) {
          gwConnected = updated;
	   if(!gwConnected)fileToClear("clear", "dashweh");
          ((SH*)screen)->log(string("GW ") + exName,/*gw->name,*/ "Quoting state changed to", string(!gwConnected?"DIS":"") + "CONNECTED");
        }
        ((UI*)client)->send(mMatter::Connectivity, semaphore());
        ((SH*)screen)->refresh();
      };
      json semaphore() {
        return {
          {"state", gwConnected},
          {"status", gwConnectedExchange}
        };
      };
      inline void timer_1s() {                                      _debugEvent_
        if (gwT_countdown and gwT_countdown-- == 1)
          gw->hub->connect(gw->ws, nullptr, {}, 5000, gw->gwGroup);
        else ((QE*)engine)->timer_1s();
        if (sync_orders and !(gwT_5m % 2))  ((EV*)events)->async(gw->orders);
        if (sync_levels and !(gwT_5m % 3))  ((EV*)events)->async(gw->levels);
        if (FN::trueOnce(&gw->forceUpdate)
          or !(gwT_5m % 15))                ((EV*)events)->async(gw->wallet);
        if (sync_trades and !(gwT_5m % 60)) ((EV*)events)->async(gw->trades);
        if (++gwT_5m == 300) {
          gwT_5m = 0;
          if (qp->cancelOrdersAuto)         ((EV*)events)->async(gw->cancelAll);
        }
      };
      inline void gwConnect(string reason = "") {                   _debugEvent_
        if (reason.empty())
          gwT_countdown = 1;
        else {
          gwT_countdown = 7;
          ((SH*)screen)->log(string("GW ") + exName,/*gw->name,*/ string("WS ") + reason + ", reconnecting in " + to_string(gwT_countdown) + "s.");
        }
      };
      inline void stunnel(bool reboot = false) {
        system("pkill stunnel || :");
        if (reboot) system("stunnel etc/stunnel.conf");
      };
      inline void handshake(mExchange k) {
        json reply;
        if (k == mExchange::Coinbase) {
          stunnel(true);
          gw->randId = FN::uuid36Id;
          gw->symbol = FN::strU(string(gw->base) + "-" + gw->quote);
          reply = FN::wJet(string(gw->http) + "/products/" + gw->symbol);
          gw->minTick = stod(reply.value("quote_increment", "0"));
          gw->minSize = stod(reply.value("base_min_size", "0"));
        }
        else if (k == mExchange::HitBtc) {
          gw->randId = FN::uuid32Id;
          gw->symbol = FN::strU(string(gw->base) + gw->quote);
          reply = FN::wJet(string(gw->http) + "/public/symbol/" + gw->symbol);
          gw->minTick = stod(reply.value("tickSize", "0"));
          gw->minSize = stod(reply.value("quantityIncrement", "0"));
          gw->base = reply.value("baseCurrency", gw->base);
          gw->quote = reply.value("quoteCurrency", gw->quote);
        }
        else if (k == mExchange::Bitfinex or k == mExchange::BitfinexMargin) {
          gw->randId = FN::int45Id;
          gw->symbol = FN::strL(string(gw->base) + gw->quote);
          reply = FN::wJet(string(gw->http) + "/pubticker/" + gw->symbol);
          if (reply.find("last_price") != reply.end()) {
            stringstream price_;
            price_ << scientific << stod(reply.value("last_price", "0"));
            string _price_ = price_.str();
            for (string::iterator it=_price_.begin(); it!=_price_.end();)
              if (*it == '+' or *it == '-') break; else it = _price_.erase(it);
            stringstream os(string("1e") + to_string(fmax(stod(_price_),-4)-4));
            os >> gw->minTick;
          }
          reply = FN::wJet(string(gw->http) + "/symbols_details");
          if (reply.is_array())
            for (json::iterator it=reply.begin(); it!=reply.end();++it)
              if (it->find("pair") != it->end() and it->value("pair", "") == gw->symbol)
                gw->minSize = stod(it->value("minimum_order_size", "0"));
        }
        else if (k == mExchange::OkCoin or k == mExchange::OkEx) {
          gw->randId = FN::char16Id;
          gw->symbol = FN::strL(string(gw->base) + "_" + gw->quote);
          gw->minTick = 0.0001;
          gw->minSize = 0.001;
        }
        else if (k == mExchange::Kraken) {
          gw->randId = FN::int32Id;
          gw->symbol = FN::strU(string(gw->base) + gw->quote);
          reply = FN::wJet(string(gw->http) + "/0/public/AssetPairs?pair=" + gw->symbol);
          if (reply.find("result") != reply.end())
            for (json::iterator it = reply["result"].begin(); it != reply["result"].end(); ++it)
              if (it.value().find("pair_decimals") != it.value().end()) {
                stringstream os(string("1e-") + to_string(it.value().value("pair_decimals", 0)));
                os >> gw->minTick;
                os = stringstream(string("1e-") + to_string(it.value().value("lot_decimals", 0)));
                os >> gw->minSize;
                gw->symbol = it.key();
                gw->base = it.value().value("base", gw->base);
                gw->quote = it.value().value("quote", gw->quote);
                break;
              }
        }
        else if (k == mExchange::Korbit) {
          gw->randId = FN::int45Id;
          gw->symbol = FN::strL(string(gw->base) + "_" + gw->quote);
          reply = FN::wJet(string(gw->http) + "/constants");
          if (reply.find(gw->symbol.substr(0,3).append("TickSize")) != reply.end()) {
            gw->minTick = reply.value(gw->symbol.substr(0,3).append("TickSize"), 0.0);
            gw->minSize = 0.015;
          }
        }
        else if (k == mExchange::Poloniex) {
          gw->randId = FN::int45Id;
          gw->symbol = FN::strU(string(gw->quote) + "_" + gw->base);
          reply = FN::wJet(string(gw->http) + "/public?command=returnTicker");
          if (reply.find(gw->symbol) != reply.end()) {
            istringstream os(string("1e-").append(to_string(6-reply[gw->symbol]["last"].get<string>().find("."))));
            os >> gw->minTick;
            gw->minSize = 0.001;
          }
        }
        else if (k == mExchange::BeaXchange) {
          gw->randId = FN::uuid36Id;
          gw->symbol = FN::strU(string(gw->base) + "_" + gw->quote);
          gw->minTick = 0.00001;
          gw->minSize = 0.00001;
        }
        if (!gw->minTick or !gw->minSize)
          exit(_redAlert_("CF", "Unable to fetch data from " + exName/*gw->name,*/
            + " for symbol \"" + gw->symbol + "\", possible error message: "
            + reply.dump(),
          true));
        if (k != mExchange::BeaXchange)
          ((SH*)screen)->log(string("GW ") + exName,/*gw->name,*/ "allows client IP");
        unsigned int precision = gw->minTick < 1e-8 ? 10 : 8;
        ((SH*)screen)->log(string("GW ") + string("BeaXchange")/*gw->name*/ + ":", string("\n")
          + "- autoBot: " + (!gwConnectedAdmin ? "no" : "yes") + '\n'
          + "- symbols: " + gw->symbol + '\n'
          + "- minTick: " + FN::strX(gw->minTick, precision) + '\n'
          + "- minSize: " + FN::strX(gw->minSize, precision) + '\n'
          + "- makeFee: " + FN::strX(gw->makeFee, precision) + '\n'
          + "- takeFee: " + FN::strX(gw->takeFee, precision));
      };
  };
}

#endif
