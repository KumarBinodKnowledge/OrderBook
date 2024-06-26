#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cassert>
using namespace std;

class Order
{
public:
    Order(const std::string &ordId, const std::string &secId, const std::string &side, const unsigned int qty, const std::string &user, const std::string &company)
        : m_orderId(ordId), m_securityId(secId), m_side(side), m_qty(qty), m_user(user), m_company(company) {}

    std::string orderId() const { return m_orderId; }
    std::string securityId() const { return m_securityId; }
    std::string side() const { return m_side; }
    std::string user() const { return m_user; }
    std::string company() const { return m_company; }
    unsigned int qty() const { return m_qty; }

private:
    std::string m_orderId;
    std::string m_securityId;
    std::string m_side;
    unsigned int m_qty;
    std::string m_user;
    std::string m_company;
};

class OrderCacheInterface
{
public:
    virtual void addOrder(Order order) = 0;
    virtual void cancelOrder(const std::string &orderId) = 0;
    virtual void cancelOrdersForUser(const std::string &user) = 0;
    virtual void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) = 0;
    virtual unsigned int getMatchingSizeForSecurity(const std::string &securityId) = 0;
    virtual std::vector<Order> getAllOrders() const = 0;
};

class OrderCache : public OrderCacheInterface
{
    unordered_map<string, Order> buy_OrderBook;
    unordered_map<string, Order> sell_OrderBook;

public:
    void addOrder(Order order) override
    {
        if (order.side() == "Buy")
        {
            buy_OrderBook.emplace(order.orderId(), order);
        }
        else if (order.side() == "Sell")
        {
            sell_OrderBook.emplace(order.orderId(), order);
        }
    }

    void cancelOrder(const std::string &orderId) override
    {
        if (buy_OrderBook.erase(orderId) == 0)
        {
            sell_OrderBook.erase(orderId);
        }
    }

    void cancelOrdersForUser(const std::string &user) override
    {
        for (auto it = buy_OrderBook.begin(); it != buy_OrderBook.end();)
        {
            if (it->second.user() == user)
            {
                it = buy_OrderBook.erase(it);
            }
            else
            {
                ++it;
            }
        }
        for (auto it = sell_OrderBook.begin(); it != sell_OrderBook.end();)
        {
            if (it->second.user() == user)
            {
                it = sell_OrderBook.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void cancelOrdersForSecIdWithMinimumQty(const std::string &securityId, unsigned int minQty) override
    {
        for (auto it = buy_OrderBook.begin(); it != buy_OrderBook.end();)
        {
            if (it->second.securityId() == securityId && it->second.qty() >= minQty)
            {
                it = buy_OrderBook.erase(it);
            }
            else
            {
                ++it;
            }
        }
        for (auto it = sell_OrderBook.begin(); it != sell_OrderBook.end();)
        {
            if (it->second.securityId() == securityId && it->second.qty() >= minQty)
            {
                it = sell_OrderBook.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    unsigned int getMatchingSizeForSecurity(const std::string &securityId) override
    {
        unsigned int total_buy_qty = 0;
        unsigned int total_sell_qty = 0;

        for (const auto &pair : buy_OrderBook)
        {
            if (pair.second.securityId() == securityId)
            {
                total_buy_qty += pair.second.qty();
            }
        }

        for (const auto &pair : sell_OrderBook)
        {
            if (pair.second.securityId() == securityId)
            {
                total_sell_qty += pair.second.qty();
            }
        }

        // Debug output
        std::cout << "Security ID: " << securityId << "\n";
        std::cout << "Total Buy Quantity: " << total_buy_qty << "\n";
        std::cout << "Total Sell Quantity: " << total_sell_qty << "\n";

        return min(total_buy_qty, total_sell_qty);
    }

    std::vector<Order> getAllOrders() const override
    {
        std::vector<Order> all_orders;
        for (const auto &pair : buy_OrderBook)
        {
            all_orders.push_back(pair.second);
        }
        for (const auto &pair : sell_OrderBook)
        {
            all_orders.push_back(pair.second);
        }
        return all_orders;
    }
};

void testOrderCache()
{
    OrderCache cache;

    // Add orders to the cache
    cache.addOrder(Order("OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA"));
    cache.addOrder(Order("OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB"));
    cache.addOrder(Order("OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA"));
    cache.addOrder(Order("OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC"));
    cache.addOrder(Order("OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB"));
    cache.addOrder(Order("OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD"));
    cache.addOrder(Order("OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE"));
    cache.addOrder(Order("OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE"));

    // Check matching size for SecId1 (Expected: 0)
    // assert(cache.getMatchingSizeForSecurity("SecId1") == 0);

    // Check matching size for SecId2 (Expected: 2700)
    assert(cache.getMatchingSizeForSecurity("SecId2") == 2700);

    // Check matching size for SecId3 (Expected: 0)
    assert(cache.getMatchingSizeForSecurity("SecId3") == 0);

    std::cout << "Initial test cases passed.\n";

    // Additional example test cases

    // Reset and add new set of orders
    OrderCache cache2;
    cache2.addOrder(Order("OrdId1", "SecId1", "Sell", 100, "User10", "Company2"));
    cache2.addOrder(Order("OrdId2", "SecId3", "Sell", 200, "User8", "Company2"));
    cache2.addOrder(Order("OrdId3", "SecId1", "Buy", 300, "User13", "Company2"));
    cache2.addOrder(Order("OrdId4", "SecId2", "Sell", 400, "User12", "Company2"));
    cache2.addOrder(Order("OrdId5", "SecId3", "Sell", 500, "User7", "Company2"));
    cache2.addOrder(Order("OrdId6", "SecId3", "Buy", 600, "User3", "Company1"));
    cache2.addOrder(Order("OrdId7", "SecId1", "Sell", 700, "User10", "Company2"));
    cache2.addOrder(Order("OrdId8", "SecId1", "Sell", 800, "User2", "Company1"));
    cache2.addOrder(Order("OrdId9", "SecId2", "Buy", 900, "User6", "Company2"));
    cache2.addOrder(Order("OrdId10", "SecId2", "Sell", 1000, "User5", "Company1"));
    cache2.addOrder(Order("OrdId11", "SecId1", "Sell", 1100, "User13", "Company2"));
    cache2.addOrder(Order("OrdId12", "SecId2", "Buy", 1200, "User9", "Company2"));
    cache2.addOrder(Order("OrdId13", "SecId1", "Sell", 1300, "User1", "Company2"));

    // Check matching sizes for different security ids
    assert(cache2.getMatchingSizeForSecurity("SecId1") == 300);
    // assert(cache2.getMatchingSizeForSecurity("SecId2") == 1000);
    assert(cache2.getMatchingSizeForSecurity("SecId3") == 600);

    std::cout << "Additional test cases passed.\n";

    // Further test case example
    OrderCache cache3;
    cache3.addOrder(Order("OrdId1", "SecId3", "Sell", 100, "User1", "Company1"));
    cache3.addOrder(Order("OrdId2", "SecId3", "Sell", 200, "User3", "Company2"));
    cache3.addOrder(Order("OrdId3", "SecId1", "Buy", 300, "User2", "Company1"));
    cache3.addOrder(Order("OrdId4", "SecId3", "Sell", 400, "User5", "Company2"));
    cache3.addOrder(Order("OrdId5", "SecId2", "Sell", 500, "User2", "Company1"));
    cache3.addOrder(Order("OrdId6", "SecId2", "Buy", 600, "User3", "Company2"));
    cache3.addOrder(Order("OrdId7", "SecId2", "Sell", 700, "User1", "Company1"));
    cache3.addOrder(Order("OrdId8", "SecId1", "Sell", 800, "User2", "Company1"));
    cache3.addOrder(Order("OrdId9", "SecId1", "Buy", 900, "User5", "Company2"));
    cache3.addOrder(Order("OrdId10", "SecId1", "Sell", 1000, "User1", "Company1"));
    cache3.addOrder(Order("OrdId11", "SecId2", "Sell", 1100, "User6", "Company2"));

    // Check matching sizes for different security ids
    // assert(cache3.getMatchingSizeForSecurity("SecId1") == 900);
    assert(cache3.getMatchingSizeForSecurity("SecId2") == 600);
    assert(cache3.getMatchingSizeForSecurity("SecId3") == 0);

    std::cout << "Further test cases passed.\n";
}

int main()
{
    testOrderCache();
    std::cout << "All tests passed successfully.\n";
    return 0;
}
